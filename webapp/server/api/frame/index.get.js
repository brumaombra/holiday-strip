import { buildMockFrame, getCurrentScene, getEspMockState, handleEspMockError } from '~~/server/utils/esp-mock-state.js';

export default defineEventHandler(async event => {
    try {
        // Draw the frame now on screen
        const state = getEspMockState();
        const frame = buildMockFrame({ sceneIndex: getCurrentScene({ state }), isOn: state.settings.on });

        // Return it as raw bytes, never cached (it changes several times per second)
        setResponseHeader(event, 'content-type', 'application/octet-stream');
        setResponseHeader(event, 'cache-control', 'no-store');
        return Buffer.from(frame);
    } catch (error) {
        return handleEspMockError({ event, error, errorId: 'mock_frame_read_failed', errorMessage: 'Failed to read the frame' });
    }
});
