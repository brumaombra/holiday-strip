import { decodeFrame } from '~/composables/useFrame.js';

// The request wrapper (fetchEsp) and the Wi-Fi and device calls come from the gizmo-common layer: only the calls of the Holiday Strip are here

// Fetch the device state: the settings, plus what the device knows about itself
export const getState = async () => {
    const response = await fetchEsp('/api/state');
    return response.data || {};
};

// Send some of the settings ({ on, brightness, scene, maxMilliamps, countdown, message }) and get the state after the change
export const saveSettings = async settings => {
    // Call the device API to save the settings
    const response = await fetchEsp('/api/settings', {
        method: 'POST',
        body: settings
    });

    // Return the state after the change
    return response.data || {};
};

// Fetch the frame on the matrix as raw bytes (72x12 pixels, 3 bytes each)
export const getFrame = async () => {
    const buffer = await fetchEsp('/api/frame', { responseType: 'arrayBuffer', retry: 0 });
    return decodeFrame(buffer);
};