import { createSuccessResponse, handleEspMockError } from 'gizmo-common/web/server/utils/gizmo-mock.js';
import { buildStatePayload, getEspMockState } from '~~/server/utils/esp-mock-state.js';

export default defineEventHandler(async event => {
    try {
        // Return the settings and what the device knows about itself, like the firmware
        return createSuccessResponse(buildStatePayload({ state: getEspMockState() }));
    } catch (error) {
        return handleEspMockError({ event, error, errorId: 'mock_state_read_failed', errorMessage: 'Failed to read the state' });
    }
});
