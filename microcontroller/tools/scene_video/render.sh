#!/usr/bin/env bash
# Render a scene of the matrix to an MP4 video, with the real scene code compiled for this computer
# Usage: tools/scene_video/render.sh HOLIDAY SCENE SECONDS OUTPUT.mp4 [YYYY-MM-DDTHH:MM:SS]
#   HOLIDAY and SCENE are indexes (0 Christmas, 1 New Year's Eve; the scenes in the order of scenes.cpp)
#   the date sets the clock the scenes read (the countdown, the year); without it the clock is unknown
#   MESSAGE_COLOR=0..6 sets the color of the custom message (white, warm white, red, green, blue, gold, rainbow)
set -euo pipefail
HERE="$(cd "$(dirname "$0")" && pwd)"
SRC="$HERE/../../src"
BUILD="${TMPDIR:-/tmp}/scene_video"
mkdir -p "$BUILD"

# Build the renderer with the scenes and the code they use
g++ -std=gnu++17 -O2 -w -I "$HERE/stub" -I "$SRC" -o "$BUILD/render" "$HERE/render.cpp" \
	"$SRC"/scenes/*.cpp "$SRC"/calendar/*.cpp "$SRC"/config/config.cpp

# Render the frames and encode them (72 x 12 LEDs at 20 video pixels each, 30 frames per second)
"$BUILD/render" "$1" "$2" "$3" ${5:+"$5"} | ffmpeg -loglevel error -y -f rawvideo -pix_fmt rgb24 -s 1440x240 -r 30 -i - \
	-c:v libx264 -pix_fmt yuv420p -crf 18 -movflags +faststart "$4"
echo "Wrote $4"
