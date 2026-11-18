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

// Holidays of the firmware, in order, each with its scenes (the id is what the web app translates, the name is the English fallback)
export const HOLIDAYS = [{
    id: 'christmas',
    name: 'Christmas',
    scenes: [
        { id: 'greeting', name: 'Greeting' },
        { id: 'snow', name: 'Snow' },
        { id: 'trees', name: 'Trees' },
        { id: 'sleigh', name: 'Sleigh' },
        { id: 'fireplace', name: 'Fireplace' },
        { id: 'garland', name: 'Garland' },
        { id: 'gifts', name: 'Gifts' },
        { id: 'candy-cane', name: 'Candy cane' }
    ]
}, {
    id: 'new-year',
    name: 'New Year\'s Eve',
    scenes: [
        { id: 'countdown', name: 'Countdown' },
        { id: 'confetti', name: 'Confetti' },
        { id: 'fireworks', name: 'Fireworks' },
        { id: 'champagne', name: 'Champagne' },
        { id: 'sparklers', name: 'Sparklers' }
    ]
}, {
    id: 'custom',
    name: 'Custom message',
    scenes: [
        { id: 'message', name: 'Message' }
    ]
}];

// Colors of the custom message: white, warm white, red, green, blue, gold, rainbow
export const MESSAGE_COLOR_COUNT = 7;

// Languages of the texts on the matrix, and the default greeting of each holiday in each of them (like the firmware)
export const LANGUAGES = ['it', 'en'];
const DEFAULT_MESSAGES = {
    it: ['BUON NATALE!', 'BUON ANNO!', 'CIAO!'],
    en: ['MERRY CHRISTMAS!', 'HAPPY NEW YEAR!', 'HELLO!']
};

// The two-language greetings, the defaults before the languages
const LEGACY_MESSAGES = ['BUON NATALE * MERRY CHRISTMAS', 'BUON ANNO * HAPPY NEW YEAR', 'CIAO! * HELLO!'];

// Check whether a greeting is still a default one, so a change of language translates it
const isDefaultMessage = ({ message, index }) => message === LEGACY_MESSAGES[index] || LANGUAGES.some(language => DEFAULT_MESSAGES[language][index] === message);

// Indexes of the holidays the calendar picks between (it never picks the custom message)
const CHRISTMAS_INDEX = 0;
const NEW_YEAR_INDEX = 1;

// Time each scene stays on screen when the device rotates them all
export const SCENE_DURATION_MS = 15000;

// Settings of the device (the firmware defaults)
const factorySettings = () => ({
    on: true,
    brightness: 60,
    holiday: -1,
    scene: -1,
    maxMilliamps: 3000,
    countdown: true,
    messages: [...DEFAULT_MESSAGES.it],
    messageColor: 0,
    language: 'it'
});

// Get the mock device state: the one of the layer (Wi-Fi, uptime, kept across hot reloads) plus the settings of the strip
export const getEspMockState = () => {
    const state = getGizmoMockState();
    if (!state.settings?.messages) state.settings = factorySettings(); // Also replaces the settings of an older shape kept across hot reloads
    return state;
};

// Get the holiday the calendar picks: New Year's Eve from December 27 to January 1 included, Christmas the rest of the year
export const getCalendarHoliday = (now = Date.now()) => {
    const date = new Date(now);
    const month = date.getMonth(); // 0 is January
    const day = date.getDate();
    const isNewYearPeriod = (month === 11 && day >= 27) || (month === 0 && day === 1);
    return isNewYearPeriod ? NEW_YEAR_INDEX : CHRISTMAS_INDEX;
};

// Get the holiday playing now: the chosen one, or the one of the calendar when the choice is Automatic (-1)
export const getActiveHoliday = ({ state, now = Date.now() }) => {
    return state.settings.holiday >= 0 ? state.settings.holiday : getCalendarHoliday(now);
};

/******************************** Settings ********************************/

// Check an integer inside a range
const isIntegerInRange = (value, minimum, maximum) => Number.isInteger(value) && value >= minimum && value <= maximum;

// Get the holiday a settings body ends up in (the holiday it sends, or the current one, resolved by the calendar when Automatic)
const getResultingHoliday = ({ state, body, now }) => {
    const holiday = body.holiday ?? state.settings.holiday;
    return holiday >= 0 ? holiday : getActiveHoliday({ state, now });
};

// Rules of each settings field, in the order the firmware checks them (a missing field keeps its current value)
const settingsRules = {
    on: value => typeof value === 'boolean',
    countdown: value => typeof value === 'boolean',
    messageColor: value => isIntegerInRange(value, 0, MESSAGE_COLOR_COUNT - 1),
    language: value => LANGUAGES.includes(value),
    brightness: value => isIntegerInRange(value, 0, 255),
    holiday: value => isIntegerInRange(value, -1, HOLIDAYS.length - 1),
    scene: (value, { state, body, now }) => isIntegerInRange(value, -1, HOLIDAYS[getResultingHoliday({ state, body, now })].scenes.length - 1),
    maxMilliamps: value => isIntegerInRange(value, MIN_MAX_MILLIAMPS, MAX_MAX_MILLIAMPS),
    messages: value => Array.isArray(value) && value.length <= HOLIDAYS.length && value.every(message => message === null || (typeof message === 'string' && byteLength(message) <= MAX_MESSAGE_LENGTH))
};

// Validate a settings body like the firmware: throws the 400 error with the name of the first wrong field, in the firmware's order (unknown and null fields are ignored)
export const validateSettings = ({ state, body, now = Date.now() }) => {
    const invalidField = Object.keys(settingsRules).find(field => body[field] !== undefined && body[field] !== null && !settingsRules[field](body[field], { state, body, now }));
    if (invalidField) {
        throw new EspMockError(400, 'invalid_settings_field', invalidField);
    }
};

// Apply a settings body that passed the validation (the device stores the messages in capitals)
export const applySettings = ({ state, body, now = Date.now() }) => {
    const { settings } = state;

    // Read the holiday playing now before changing anything (a scene sent while Automatic pins this one)
    const activeHoliday = getActiveHoliday({ state, now });

    // Apply the plain fields (null is the same as a missing field, like on the firmware)
    for (const field of ['on', 'countdown', 'brightness', 'maxMilliamps', 'messageColor']) {
        if (body[field] === undefined || body[field] === null) continue;
        settings[field] = body[field];
    }

    // Change the holiday: the scenes of another one are different, so the scene goes back to rotating unless one is sent (the same holiday again keeps it, like the firmware)
    if (body.holiday !== undefined && body.holiday !== null && body.holiday !== settings.holiday) {
        settings.holiday = body.holiday;
        settings.scene = -1;
    }

    // Change the scene: a specific scene only makes sense inside a fixed holiday, so Automatic is pinned to the holiday playing now
    if (body.scene !== undefined && body.scene !== null) {
        settings.scene = body.scene;
        if (body.scene >= 0 && settings.holiday < 0) settings.holiday = activeHoliday;
    }

    // Change the language, translating the greetings still at their default (the ones written by hand stay)
    if (body.language !== undefined && body.language !== null && body.language !== settings.language) {
        settings.messages = settings.messages.map((message, index) => isDefaultMessage({ message, index }) ? DEFAULT_MESSAGES[body.language][index] : message);
        settings.language = body.language;
    }

    // Change the messages of the holidays that have one (null leaves it as it is)
    if (body.messages) {
        body.messages.forEach((message, index) => {
            if (message !== null) settings.messages[index] = message.toUpperCase();
        });
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

    // Rotate through the scenes of the holiday playing now
    const sceneCount = HOLIDAYS[getActiveHoliday({ state, now })].scenes.length;
    return Math.floor((now - state.startedAt) / SCENE_DURATION_MS) % sceneCount;
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
        activeHoliday: getActiveHoliday({ state, now }),
        holidays: HOLIDAYS,
        currentScene: sceneIndex,
        estimatedMilliamps: estimateMilliamps({ frame, brightness: state.settings.brightness, maxMilliamps: state.settings.maxMilliamps }),
        minMaxMilliamps: MIN_MAX_MILLIAMPS,
        maxMaxMilliamps: MAX_MAX_MILLIAMPS,
        maxMessageLength: MAX_MESSAGE_LENGTH
    };
};
