#!/usr/bin/env bash
# Make the README banner (docs/images/banner-light.webp and banner-dark.webp), in the style of the ui-vintage banner
# Needs g++ and ffmpeg (the scene video tool), Node.js with Playwright and its Chromium, and webapp/node_modules (npm ci) for the icons
# Usage: tools/banner/make_banner.sh
set -euo pipefail
HERE="$(cd "$(dirname "$0")" && pwd)"
ROOT="$HERE/../.."
WORK="${TMPDIR:-/tmp}/holiday-strip-banner"
mkdir -p "$WORK"

# A real frame of the matrix for the preview card: the Christmas trees scene, a few seconds in
"$ROOT/microcontroller/tools/scene_video/render.sh" 0 2 4 "$WORK/trees.mp4" 2026-12-10T19:00:00
ffmpeg -loglevel error -y -ss 3.5 -i "$WORK/trees.mp4" -frames:v 1 "$WORK/matrix.png"

# The banner in both themes, at 2560 x 880
node "$HERE/banner.mjs" "$WORK/matrix.png" "$WORK"
for theme in light dark; do
	ffmpeg -loglevel error -y -i "$WORK/banner-$theme.png" -c:v libwebp -quality 90 "$ROOT/docs/images/banner-$theme.webp"
done
echo "Wrote docs/images/banner-light.webp and banner-dark.webp"
