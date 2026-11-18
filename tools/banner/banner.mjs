// Draw the README banner in the style of the ui-vintage banner (its colors, borders, monospace and hugeicons), and screenshot it in the light and dark theme
// Run by make_banner.sh: node banner.mjs MATRIX.png OUTPUT_DIR
import { readFileSync } from 'node:fs';
import { createRequire } from 'node:module';
import path from 'node:path';
import { fileURLToPath, pathToFileURL } from 'node:url';

const require = createRequire(import.meta.url);
const { chromium } = require('playwright');

const HERE = path.dirname(fileURLToPath(import.meta.url));
const ICONS_DIR = path.join(HERE, '../../webapp/node_modules/@hugeicons/core-free-icons/dist/esm');
const [matrixPath, outputDir] = process.argv.slice(2);

// Load a hugeicons icon as an SVG string (the same icons as the dashboard)
const loadIcon = async name => {
	const { default: icon } = await import(pathToFileURL(path.join(ICONS_DIR, `${name}.js`)).href);
	const toAttribute = key => key.replace(/[A-Z]/g, letter => `-${letter.toLowerCase()}`);
	const elements = icon.map(([tag, attributes]) => `<${tag} ${Object.entries(attributes).filter(([key]) => key !== 'key').map(([key, value]) => `${toAttribute(key)}="${value}"`).join(' ')}/>`);
	return `<svg viewBox="0 0 24 24" fill="none" xmlns="http://www.w3.org/2000/svg">${elements.join('')}</svg>`;
};

// The icons of the cards
const icons = {
	calendar: await loadIcon('Calendar03Icon'),
	sparkles: await loadIcon('SparklesIcon'),
	power: await loadIcon('PowerIcon'),
	sun: await loadIcon('Sun01Icon'),
	check: await loadIcon('CheckmarkCircle02Icon'),
	tree: await loadIcon('PineTreeIcon'),
	fireworks: await loadIcon('FireworksIcon'),
	chevron: await loadIcon('ArrowRight01Icon')
};
const logo = readFileSync(path.join(HERE, '../../webapp/public/logo.svg'), 'utf8');
const matrix = `data:image/png;base64,${readFileSync(matrixPath).toString('base64')}`;

// The page: the ui-vintage tokens for each theme, the title on the left, tilted cards of the dashboard on the right
const page = theme => `<!doctype html>
<html><head><meta charset="utf-8"><style>
	:root {
		--bg: ${theme === 'light' ? '#fafafa' : '#0f1318'};
		--card: ${theme === 'light' ? '#ffffff' : '#171c23'};
		--fg: ${theme === 'light' ? '#1b222d' : '#eef3f8'};
		--muted: ${theme === 'light' ? '#606976' : '#97a2b2'};
		--border: ${theme === 'light' ? '#d8dee7' : '#313948'};
		--grid: ${theme === 'light' ? 'rgb(27 34 45 / 0.05)' : 'rgb(238 243 248 / 0.05)'};
		--primary: ${theme === 'light' ? '#f7931a' : '#f7a23c'};
		--track: ${theme === 'light' ? '#edf1f5' : '#202733'};
		--success: ${theme === 'light' ? '#15803d' : '#4ade80'};
		--success-bg: ${theme === 'light' ? '#ffffff' : '#132119'};
		--success-border: ${theme === 'light' ? '#bbe5c9' : '#1f5133'};
		--shadow: ${theme === 'light' ? '0 2px 4px rgb(16 24 40 / 0.04), 0 12px 32px -8px rgb(16 24 40 / 0.16)' : '0 12px 32px -8px rgb(0 0 0 / 0.6)'};
	}
	* { box-sizing: border-box; margin: 0; }
	body { width: 1280px; height: 440px; overflow: hidden; background: var(--bg); color: var(--fg); font-family: 'JetBrains Mono', ui-monospace, monospace;
		background-image: linear-gradient(var(--grid) 1px, transparent 1px), linear-gradient(90deg, var(--grid) 1px, transparent 1px); background-size: 24px 24px; position: relative; }
	.accent { position: absolute; left: 0; bottom: 0; width: 160px; height: 4px; background: var(--primary); }
	.left { position: absolute; left: 64px; top: 58px; width: 560px; }
	.brand { display: flex; align-items: center; gap: 18px; }
	.brand .logo { width: 60px; height: 60px; border-radius: 10px; overflow: hidden; ${theme === 'dark' ? 'box-shadow: 0 0 0 1px var(--border);' : ''} }
	.brand .logo svg { width: 100%; height: 100%; display: block; }
	.brand .label { font-size: 12.5px; letter-spacing: 0.42em; color: var(--muted); }
	h1 { font-size: 72px; font-weight: 700; line-height: 1; margin-top: 30px; letter-spacing: -0.01em; white-space: nowrap; }
	h1 .strip { color: var(--primary); position: relative; display: inline-block; }
	h1 .strip::after { content: ''; position: absolute; left: -2px; right: -2px; bottom: -14px; height: 4px; border-radius: 4px; background: var(--primary); opacity: 0.55; transform: rotate(-0.6deg); }
	.lead { margin-top: 36px; font-size: 16px; line-height: 1.75; color: var(--muted); }
	.chips { display: flex; gap: 10px; margin-top: 28px; }
	.chip { border: 1px solid var(--border); background: var(--card); border-radius: 4px; padding: 7px 12px; font-size: 13.5px; }
	.chip.command { padding: 8px 14px; }
	.chip.command b { color: var(--primary); font-weight: 400; margin-right: 6px; }
	.card { position: absolute; background: var(--card); border: 1px solid var(--border); border-radius: 4px; box-shadow: var(--shadow); }
	.card .title { font-size: 11px; letter-spacing: 0.16em; color: var(--muted); display: flex; align-items: center; gap: 8px; }
	.dot { width: 7px; height: 7px; border-radius: 50%; background: #ef4444; box-shadow: 0 0 0 3px rgb(239 68 68 / 0.18); }
	.row { display: flex; align-items: center; gap: 12px; }
	.icon { width: 18px; height: 18px; color: var(--muted); flex: none; }
	.icon svg { width: 100%; height: 100%; display: block; }
	.value { margin-left: auto; color: var(--muted); font-size: 13px; display: flex; align-items: center; gap: 6px; }
	.value .icon { width: 14px; height: 14px; }
	.name { font-weight: 700; font-size: 15px; }
	.sub { color: var(--muted); font-size: 12px; margin-top: 3px; }
	.switch { margin-left: auto; width: 46px; height: 26px; border: 1px solid var(--primary); border-radius: 4px; background: var(--track); position: relative; }
	.switch::after { content: ''; position: absolute; right: 4px; top: 4px; width: 16px; height: 16px; border-radius: 3px; background: var(--primary); }
	.slider { height: 6px; border-radius: 6px; background: var(--track); position: relative; margin-top: 14px; }
	.slider .fill { position: absolute; left: 0; top: 0; bottom: 0; width: 38%; border-radius: 6px; background: var(--primary); }
	.slider .thumb { position: absolute; left: calc(38% - 9px); top: -7px; width: 20px; height: 20px; border: 2px solid var(--primary); background: var(--card); border-radius: 4px; }
	.divider { height: 1px; background: var(--border); margin: 16px -20px; }
	.toast { background: var(--success-bg); border-color: var(--success-border); color: var(--success); }
	.toast .icon { color: var(--success); width: 20px; height: 20px; }
	.stats { display: flex; }
	.stats > div { display: flex; align-items: center; gap: 7px; padding: 11px 12px; font-size: 13px; }
	.stats > div + div { border-left: 1px solid var(--border); }
	.stats .icon { color: var(--primary); }
	.stats b { font-weight: 700; }
	.stats span { color: var(--muted); }
</style></head><body>
	<div class="left">
		<div class="brand"><div class="logo">${logo}</div><div class="label">ESP32 LED MATRIX</div></div>
		<h1>Holiday <span class="strip">Strip</span></h1>
		<p class="lead">Hand-drawn holiday scenes on a 72x12 truck LED matrix, with countdowns and a dashboard built into the firmware.</p>
		<div class="chips">
			<div class="chip command"><b>$</b>pio run -t upload</div>
			<div class="chip">ESP32</div>
			<div class="chip">864 LEDs</div>
			<div class="chip">Nuxt 4</div>
		</div>
	</div>

	<!-- The live preview, with a real frame of the matrix -->
	<div class="card" style="left: 676px; top: 40px; width: 560px; padding: 16px 18px 18px; transform: rotate(-1.6deg);">
		<div class="title"><span class="dot"></span>LIVE PREVIEW</div>
		<img src="${matrix}" style="display: block; width: 100%; margin-top: 12px; border-radius: 3px;">
	</div>

	<!-- The holiday picker and the display -->
	<div class="card" style="left: 712px; top: 196px; width: 300px; padding: 16px 20px; transform: rotate(1.2deg);">
		<div class="row"><div class="icon">${icons.calendar}</div><div><div class="name">Holiday</div><div class="sub">Now: Christmas</div></div><div class="value">Automatic <span class="icon">${icons.chevron}</span></div></div>
		<div class="divider"></div>
		<div class="row"><div class="icon">${icons.power}</div><div class="name">Display on</div><div class="switch"></div></div>
		<div class="divider"></div>
		<div class="row"><div class="icon">${icons.sun}</div><div class="name">Brightness</div><div class="value">38%</div></div>
		<div class="slider"><div class="fill"></div><div class="thumb"></div></div>
	</div>

	<!-- A saved message -->
	<div class="card toast" style="left: 1036px; top: 212px; width: 210px; padding: 16px 18px; transform: rotate(-1deg);">
		<div class="row" style="align-items: flex-start;"><div class="icon">${icons.check}</div><div style="font-size: 14px; line-height: 1.5;">37 days to Christmas!</div></div>
	</div>

	<!-- The holidays -->
	<div class="card stats" style="left: 1040px; top: 334px; transform: rotate(0.8deg);">
		<div><span class="icon">${icons.tree}</span><b>9</b><span>scenes</span></div>
		<div><span class="icon">${icons.fireworks}</span><b>5</b><span>scenes</span></div>
	</div>

	<div class="accent"></div>
</body></html>`;

// Screenshot both themes at twice the size, for sharp text
const browser = await chromium.launch();
for (const theme of ['light', 'dark']) {
	const tab = await browser.newPage({ viewport: { width: 1280, height: 440 }, deviceScaleFactor: 2 });
	await tab.setContent(page(theme), { waitUntil: 'load' });
	await tab.screenshot({ path: path.join(outputDir, `banner-${theme}.png`) });
	await tab.close();
}
await browser.close();