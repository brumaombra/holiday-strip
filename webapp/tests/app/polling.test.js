import { afterEach, beforeEach, describe, expect, it, vi } from 'vitest';
import { createPoller } from '~/composables/usePolling.js';

// Set the visibility of the page and tell the listeners
const setVisibility = state => {
    Object.defineProperty(document, 'visibilityState', { configurable: true, get: () => state });
    document.dispatchEvent(new Event('visibilitychange'));
};

// The poller of the live preview: it runs while the page is visible, and not otherwise
describe('createPoller', () => {
    // Use fake timers and a visible page
    beforeEach(() => {
        vi.useFakeTimers();
        setVisibility('visible');
    });

    // Leave the real timers and a visible page for the other tests
    afterEach(() => {
        vi.useRealTimers();
        setVisibility('visible');
    });

    // The preview must not wait a full interval to show something
    it('calls right away, then at each interval', async () => {
        const callback = vi.fn(async () => {});
        const poller = createPoller({ callback, intervalMs: 200 });
        poller.start();
        await vi.advanceTimersByTimeAsync(0);
        expect(callback).toHaveBeenCalledTimes(1);
        await vi.advanceTimersByTimeAsync(200);
        expect(callback).toHaveBeenCalledTimes(2);
        await vi.advanceTimersByTimeAsync(400);
        expect(callback).toHaveBeenCalledTimes(4);
        poller.stop();
    });

    // A slow device used to get more and more requests at the same time
    it('waits for a slow call before scheduling the next one', async () => {
        const callback = vi.fn(() => new Promise(resolve => setTimeout(resolve, 500)));
        const poller = createPoller({ callback, intervalMs: 200 });
        poller.start();
        await vi.advanceTimersByTimeAsync(450);
        expect(callback).toHaveBeenCalledTimes(1);
        await vi.advanceTimersByTimeAsync(200);
        expect(callback).toHaveBeenCalledTimes(1);
        await vi.advanceTimersByTimeAsync(100);
        expect(callback).toHaveBeenCalledTimes(2);
        poller.stop();
    });

    // A failed read (the device rebooting) must not end the preview
    it('keeps polling after a failed call', async () => {
        const callback = vi.fn().mockRejectedValueOnce(new Error('offline')).mockResolvedValue(undefined);
        const poller = createPoller({ callback, intervalMs: 200 });
        poller.start();
        await vi.advanceTimersByTimeAsync(450);
        expect(callback).toHaveBeenCalledTimes(3);
        poller.stop();
    });

    // A hidden tab used to keep reading the frame in the background
    it('stops while the page is hidden and resumes when it is visible again', async () => {
        const callback = vi.fn(async () => {});
        const poller = createPoller({ callback, intervalMs: 200 });
        poller.start();
        await vi.advanceTimersByTimeAsync(0);
        expect(callback).toHaveBeenCalledTimes(1);

        // Hide the page: nothing more is called
        setVisibility('hidden');
        await vi.advanceTimersByTimeAsync(2000);
        expect(callback).toHaveBeenCalledTimes(1);

        // Show it again: it reads right away, and goes on
        setVisibility('visible');
        await vi.advanceTimersByTimeAsync(0);
        expect(callback).toHaveBeenCalledTimes(2);
        await vi.advanceTimersByTimeAsync(200);
        expect(callback).toHaveBeenCalledTimes(3);
        poller.stop();
    });

    // Leaving the page must end the polling for good
    it('stops for good when stopped', async () => {
        const callback = vi.fn(async () => {});
        const poller = createPoller({ callback, intervalMs: 200 });
        poller.start();
        await vi.advanceTimersByTimeAsync(0);
        poller.stop();
        await vi.advanceTimersByTimeAsync(2000);
        expect(callback).toHaveBeenCalledTimes(1);

        // A page that becomes visible after the stop doesn't wake it up
        setVisibility('hidden');
        setVisibility('visible');
        await vi.advanceTimersByTimeAsync(2000);
        expect(callback).toHaveBeenCalledTimes(1);
    });

    // Starting twice must not double the requests
    it('ignores a second start', async () => {
        const callback = vi.fn(async () => {});
        const poller = createPoller({ callback, intervalMs: 200 });
        poller.start();
        poller.start();
        await vi.advanceTimersByTimeAsync(0);
        expect(callback).toHaveBeenCalledTimes(1);
        poller.stop();
    });
});
