import { beforeEach, describe, expect, it, vi } from 'vitest';
import { createDebounced } from '~/composables/useUtils.js';

// Record the error dialogs instead of rendering them
const dialogs = vi.hoisted(() => []);
vi.mock('@brumaombra/ui-vintage/message-dialog', () => ({
    showMessageDialog: options => {
        dialogs.push(options);
    }
}));

// Errors of the device: handleBackendErrors comes from the gizmo-common layer, which translates its own errors and then the ones of this app
describe('handleBackendErrors', () => {
    // Start without dialogs, and keep the logged errors out of the test output
    beforeEach(() => {
        dialogs.length = 0;
        vi.spyOn(console, 'error').mockImplementation(() => {});
    });

    // The refusal of a setting is explained in the words of this app (api.backendErrors), not shown as the name of the field
    it('translates the refusal of a setting', () => {
        const error = { data: { errorId: 'invalid_settings_field', errorMessage: 'brightness', data: null } };
        handleBackendErrors({ error, showDialog: true });
        expect(dialogs[0].message).toContain('does not accept');
        expect(dialogs[0].type).toBe('error');
    });

    // An error of the layer is translated in the app too (the layer's words are merged with the ones of the app)
    it('translates an error of the layer', () => {
        const error = { data: { errorId: 'invalid_connection_field', errorMessage: 'ssid', data: null } };
        handleBackendErrors({ error, showDialog: true });
        expect(dialogs[0].message).toContain('32 characters');
    });
});

// The slider saves wait for the slider to rest
describe('createDebounced', () => {
    // Use fake timers
    beforeEach(() => {
        vi.useFakeTimers();
    });

    // A slider drag used to send a request for every step
    it('calls once, with the last arguments, when the calls stop', () => {
        const callback = vi.fn();
        const debounced = createDebounced(callback, 300);
        debounced(1);
        vi.advanceTimersByTime(200);
        debounced(2);
        vi.advanceTimersByTime(200);
        debounced(3);
        expect(callback).not.toHaveBeenCalled();
        vi.advanceTimersByTime(300);
        expect(callback).toHaveBeenCalledTimes(1);
        expect(callback).toHaveBeenCalledWith(3);
        vi.useRealTimers();
    });

    // Leaving the page sends what is waiting, once
    it('runs the pending call right away on flush, and only once', () => {
        const callback = vi.fn();
        const debounced = createDebounced(callback, 300);
        debounced('a');
        debounced.flush();
        debounced.flush();
        vi.advanceTimersByTime(1000);
        expect(callback).toHaveBeenCalledTimes(1);
        expect(callback).toHaveBeenCalledWith('a');
        vi.useRealTimers();
    });

    // A dropped call never runs
    it('drops the pending call on cancel', () => {
        const callback = vi.fn();
        const debounced = createDebounced(callback, 300);
        debounced('a');
        debounced.cancel();
        vi.advanceTimersByTime(1000);
        expect(callback).not.toHaveBeenCalled();
        vi.useRealTimers();
    });
});