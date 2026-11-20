import { decodeFrame } from '~/composables/useFrame.js';

// The request wrapper (fetchEsp) and the Wi-Fi and device calls come from the gizmo-common layer: only the calls of the Holiday Strip are here

// Read the data of an answer, or throw when it isn't one: $fetch hands back the raw text of a body it can't parse as JSON, and taking that as an empty state would show (and save) the store defaults
// The error has no message, so the caller's default message is the one shown
const readData = response => {
    const data = response?.data;
    if (!data || typeof data !== 'object' || Array.isArray(data)) {
        throw new Error();
    }
    return data;
};

// Fetch the device state: the settings, plus what the device knows about itself
export const getState = async () => {
    const response = await fetchEsp('/api/state');
    return readData(response);
};

// Send some of the settings ({ on, brightness, holiday, scene, maxMilliamps, messages, messageColor, language }, where messages has one entry per holiday: a string to change it, null to leave it) and get the state after the change
export const saveSettings = async settings => {
    // Call the device API to save the settings
    const response = await fetchEsp('/api/settings', {
        method: 'POST',
        body: settings
    });

    // Return the state after the change
    return readData(response);
};

// Fetch the frame on the matrix as raw bytes (72x12 pixels, 3 bytes each)
export const getFrame = async () => {
    const buffer = await fetchEsp('/api/frame', { responseType: 'arrayBuffer', retry: 0 });
    return decodeFrame(buffer);
};