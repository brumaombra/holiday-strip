import { beforeEach, describe, expect, it, vi } from 'vitest';
import { createDebounced, handleBackendErrors, splitUptime } from '~/composables/useUtils.js';

// Record the error dialogs instead of rendering them
const dialogs = vi.hoisted(() => []);
vi.mock('@brumaombra/ui-vintage/message-dialog', () => ({
    showMessageDialog: options => {
        dialogs.push(options);
    }
}));

// Errors of the device: one dialog, with the translated message when the error ID is known
describe('handleBackendErrors', () => {
    // Start without dialogs, and keep the logged errors out of the test output
    beforeEach(() => {
        dialogs.length = 0;
        vi.spyOn(console, 'error').mockImplementation(() => {});
    });

    // It used to throw the error again after the dialog, which ended every failed action with an unhandled error
    it('shows the error without throwing it again', () => {
        const error = Object.assign(new Error('Bad request'), { statusCode: 400, data: { errorId: null, errorMessage: 'brightness', data: null } });
        expect(() => handleBackendErrors({ error, defaultMessage: 'Could not save', showDialog: true })).not.toThrow();
        expect(dialogs).toHaveLength(1);
    });

    // A known error ID of the firmware is shown in the user's words, not as the raw message ("ssid")
    it('translates a known error ID of the device', () => {
        const error = { data: { errorId: 'invalid_connection_field', errorMessage: 'ssid', data: null } };
        handleBackendErrors({ error, showDialog: true });
        expect(dialogs[0].message).toContain('32 characters');
        expect(dialogs[0].type).toBe('error');
    });

    // The refusal of a setting is explained, not shown as the name of the field
    it('translates the refusal of a setting', () => {
        const error = { data: { errorId: 'invalid_settings_field', errorMessage: 'brightness', data: null } };
        handleBackendErrors({ error, showDialog: true });
        expect(dialogs[0].message).toContain('does not accept');
    });

    // Without an error ID, the device message, then the default message, is shown
    it('falls back to the message of the device, then to the default one', () => {
        handleBackendErrors({ error: { data: { errorId: null, errorMessage: 'Device busy' } }, showDialog: true });
        handleBackendErrors({ error: {}, defaultMessage: 'Could not save', showDialog: true });
        expect(dialogs.map(dialog => dialog.message)).toEqual(['Device busy', 'Could not save']);
    });

    // Without showDialog, the error is only logged
    it('shows no dialog unless asked', () => {
        handleBackendErrors({ error: new Error('Offline') });
        expect(dialogs).toHaveLength(0);
        expect(console.error).toHaveBeenCalled();
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

// Uptime in the device info
describe('splitUptime', () => {
    // Days, hours and minutes of a long uptime
    it('splits the seconds in days, hours and minutes', () => {
        expect(splitUptime((2 * 86400) + (3 * 3600) + (4 * 60) + 59)).toEqual({ days: 2, hours: 3, minutes: 4 });
    });

    // A device that just booted
    it('shows zero for a device that just started', () => {
        expect(splitUptime(12)).toEqual({ days: 0, hours: 0, minutes: 0 });
    });

    // A missing or wrong uptime is zero
    it('treats a missing or wrong value as zero', () => {
        expect(splitUptime(undefined)).toEqual({ days: 0, hours: 0, minutes: 0 });
        expect(splitUptime(-5)).toEqual({ days: 0, hours: 0, minutes: 0 });
        expect(splitUptime('abc')).toEqual({ days: 0, hours: 0, minutes: 0 });
    });
});
