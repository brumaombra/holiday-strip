import { createSuccessResponse, handleEspMockError, readMockJsonBody } from 'gizmo-common/web/server/utils/gizmo-mock.js';
import { applySettings, buildStatePayload, getEspMockState, validateSettings } from '~~/server/utils/esp-mock-state.js';

export default defineEventHandler(async event => {
    try {
        // Get the request body
        const body = await readMockJsonBody(event);

        // Validate the fields in the firmware order (nothing is applied if one is wrong)
        validateSettings(body);

        // Apply only the settings fields that are present
        const state = getEspMockState();
        applySettings({ state, body });

        // Return the state after the change, like the firmware
        return createSuccessResponse(buildStatePayload({ state }));
    } catch (error) {
        return handleEspMockError({ event, error, errorId: 'settings_save_failed', errorMessage: 'Failed to save settings' });
    }
});
