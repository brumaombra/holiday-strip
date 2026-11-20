import { decodeFrame } from '~/composables/useFrame.js';

// The request wrapper (fetchEsp), readData and the Wi-Fi and device calls come from the gizmo-common layer: only the calls of the Holiday Strip are here (readData throws when an answer has no data, so a broken answer is never taken as an empty state, which would show and save the store defaults)

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