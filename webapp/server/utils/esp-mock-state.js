// The Wi-Fi and device mock (its state, the responses and errors of the firmware) comes from the gizmo-common layer: this file adds the settings and the frame of the strip
import { EspMockError, byteLength, getGizmoMockState } from 'gizmo-common/web/server/utils/gizmo-mock.js';

// Limits of the firmware (Holiday Strip settings), counted in bytes like the C strings on the device
export const MAX_MESSAGE_LENGTH = 200;
export const MIN_MAX_MILLIAMPS = 1000;
export const MAX_MAX_MILLIAMPS = 8000;

// Size of the matrix, and of its frame (3 bytes per pixel: R, G, B)
export const FRAME_WIDTH = 72;
export const FRAME_HEIGHT = 12;
export const FRAME_BYTES = FRAME_WIDTH * FRAME_HEIGHT * 3;

// Scenes of the firmware, in order
export const SCENE_NAMES = ['Greeting', 'Snowfall', 'Christmas trees', 'Santa\'s sleigh', 'Fireplace', 'Fairy lights', 'Gifts', 'Candy cane', 'Fireworks'];

// Time each scene stays on screen when the device rotates them all
export const SCENE_DURATION_MS = 15000;

// Settings of the device (the firmware defaults)
const factorySettings = () => ({
    on: true,
    brightness: 60,
    scene: -1,
    maxMilliamps: 3000,
    countdown: true,
    message: 'BUON NATALE * MERRY CHRISTMAS'
});

// Get the mock device state: the one of the layer (Wi-Fi, uptime, kept across hot reloads) plus the settings of the strip
export const getEspMockState = () => {
    const state = getGizmoMockState();
    state.settings ??= factorySettings();
    return state;
};

/******************************** Settings ********************************/

// Check an integer inside a range
const isIntegerInRange = (value, minimum, maximum) => Number.isInteger(value) && value >= minimum && value <= maximum;

// Rules of each settings field, in the order the firmware checks them (a missing field keeps its current value)
const settingsRules = {
    on: value => typeof value === 'boolean',
    countdown: value => typeof value === 'boolean',
    brightness: value => isIntegerInRange(value, 0, 255),
    scene: value => isIntegerInRange(value, -1, SCENE_NAMES.length - 1),
    maxMilliamps: value => isIntegerInRange(value, MIN_MAX_MILLIAMPS, MAX_MAX_MILLIAMPS),
    message: value => typeof value === 'string' && byteLength(value) <= MAX_MESSAGE_LENGTH
};

// Validate a settings body like the firmware: throws the 400 error with the name of the first wrong field, in the firmware's order (unknown and null fields are ignored)
export const validateSettings = body => {
    const invalidField = Object.keys(settingsRules).find(field => body[field] !== undefined && body[field] !== null && !settingsRules[field](body[field]));
    if (invalidField) {
        throw new EspMockError(400, 'invalid_settings_field', invalidField);
    }
};

// Apply a settings body that passed the validation (the device stores the message in capitals)
export const applySettings = ({ state, body }) => {
    for (const field of Object.keys(settingsRules)) {
        if (body[field] === undefined || body[field] === null) continue; // Null is the same as a missing field, like on the firmware
        state.settings[field] = field === 'message' ? body.message.toUpperCase() : body[field];
    }
};

/******************************** Frame ********************************/

// Color of the garland of each scene (the mock does not draw the real scenes, it only makes the preview move)
const sceneColors = [
    [255, 200, 40], [150, 200, 255], [30, 200, 70], [230, 40, 50], [255, 110, 20],
    [255, 180, 80], [200, 40, 200], [255, 60, 80], [90, 140, 255]
];

// Get the scene on screen: the selected one, or the one the rotation has reached
export const getCurrentScene = ({ state, now = Date.now() }) => {
    if (state.settings.scene >= 0) return state.settings.scene;
    return Math.floor((now - state.startedAt) / SCENE_DURATION_MS) % SCENE_NAMES.length;
};

// Build an animated fake frame: falling snow over a moving garland in the color of the scene (all black when the device is off)
export const buildMockFrame = ({ sceneIndex, now = Date.now(), isOn = true }) => {
    const frame = new Uint8Array(FRAME_BYTES);
    if (!isOn) return frame;

    const seconds = now / 1000;
    const [red, green, blue] = sceneColors[sceneIndex % sceneColors.length];

    for (let x = 0; x < FRAME_WIDTH; x += 1) {
        // Garland: a wave on the lower rows
        const waveRow = 9 + Math.round(Math.sin((x / 5) + (seconds * 3)) * 1.4);
        const waveLevel = 0.55 + (0.45 * Math.sin((x / 2) - (seconds * 5)));

        // Snowflake: one per column, falling at its own speed
        const speed = 2 + ((x * 7) % 5);
        const snowRow = Math.floor((seconds * speed) + ((x * 13) % FRAME_HEIGHT)) % FRAME_HEIGHT;

        for (let y = 0; y < FRAME_HEIGHT; y += 1) {
            const offset = ((y * FRAME_WIDTH) + x) * 3;
            if (y === waveRow) {
                frame[offset] = Math.round(red * waveLevel);
                frame[offset + 1] = Math.round(green * waveLevel);
                frame[offset + 2] = Math.round(blue * waveLevel);
            } else if (y === snowRow && x % 3 === 0) {
                frame[offset] = 220;
                frame[offset + 1] = 230;
                frame[offset + 2] = 255;
            }
        }
    }

    return frame;
};

// Estimate the current of the LEDs (20 mA per color at full level, scaled by the brightness and capped by the power limit, like the firmware limiter)
export const estimateMilliamps = ({ frame, brightness, maxMilliamps }) => {
    const levelsSum = frame.reduce((sum, level) => sum + level, 0);
    return Math.min(maxMilliamps, Math.round((levelsSum / 255) * 20 * (brightness / 255)));
};

// Build the state the dashboard reads (the settings plus what the device knows about itself)
export const buildStatePayload = ({ state, now = Date.now() }) => {
    const sceneIndex = getCurrentScene({ state, now });
    const frame = buildMockFrame({ sceneIndex, now, isOn: state.settings.on });
    return {
        ...state.settings,
        scenes: [...SCENE_NAMES],
        currentScene: sceneIndex,
        estimatedMilliamps: estimateMilliamps({ frame, brightness: state.settings.brightness, maxMilliamps: state.settings.maxMilliamps }),
        minMaxMilliamps: MIN_MAX_MILLIAMPS,
        maxMaxMilliamps: MAX_MAX_MILLIAMPS,
        maxMessageLength: MAX_MESSAGE_LENGTH,
        uptimeSeconds: Math.floor((now - state.startedAt) / 1000),
        ssid: state.savedSsid
    };
};
