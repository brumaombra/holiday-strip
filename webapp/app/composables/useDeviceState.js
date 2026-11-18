import { setBusy } from '@brumaombra/ui-vintage/busy-indicator';
import { getState, saveSettings } from '~/composables/useDeviceApi.js';
import { createDebounced } from '~/composables/useUtils.js';
import { useGlobalStore } from '~/composables/stores/useGlobalStore.js';

// Fields the device changes by itself, and the page only reads
const LIVE_FIELDS = ['activeHoliday', 'currentScene', 'estimatedMilliamps'];

// Load the device state into the store, and remember a failure (so the pages never show or save the store defaults)
export const loadDeviceState = async () => {
    const globalStore = useGlobalStore();

    try {
        // Get the device state
        const state = await getState();

        // Merge it with the store and mark it as loaded
        globalStore.value.device = {
            ...globalStore.value.device,
            ...state
        };
        globalStore.value.deviceLoaded = true;
        globalStore.value.deviceLoadFailed = false;
    } catch (error) {
        // Remember the failure, so the pages offer to try again
        globalStore.value.deviceLoadFailed = true;
        throw error;
    }
};

// Load the device state with the busy indicator, and tell the user when the device doesn't answer (the app shell and the retry button use it)
export const syncDeviceState = async () => {
    try {
        setBusy(true, { label: translate('common.syncingDevice') });
        await loadDeviceState();
    } catch (error) {
        handleBackendErrors({ error, defaultMessage: translate('app.loadError'), showDialog: true });
    } finally {
        setBusy(false);
    }
};

// Refresh only what the device changes by itself (the holiday playing, the scene on screen, the estimated current), so a setting being edited is never overwritten
export const refreshLiveState = async () => {
    const globalStore = useGlobalStore();
    const state = await getState();

    // Copy the live fields into the store
    for (const field of LIVE_FIELDS) {
        if (state[field] !== undefined) globalStore.value.device[field] = state[field];
    }
};

// Guess what the device will do with a change, so the page can show it right away (the answer of the device then replaces the guess)
const predictChanges = ({ changes, device }) => {
    const shown = { ...changes };

    // Merge the messages into the ones shown (a null leaves the message as it is)
    if (changes.messages) {
        shown.messages = device.messages.map((message, index) => changes.messages[index] ?? message);
    }

    // A new holiday goes back to rotating its scenes, and a fixed one is the one playing
    if (changes.holiday !== undefined) {
        if (changes.scene === undefined) shown.scene = -1;
        if (changes.holiday >= 0) shown.activeHoliday = changes.holiday;
    }

    // A specific scene pins the holiday playing now
    if (changes.scene >= 0 && (shown.holiday ?? device.holiday) < 0) {
        shown.holiday = device.activeHoliday;
    }

    // Return what to show
    return shown;
};

// Change some settings: the page shows them right away, the device confirms them, and a refusal brings back what the device really has
export const applyDeviceSettings = async changes => {
    const globalStore = useGlobalStore();

    // Show the change right away
    const shown = predictChanges({ changes, device: globalStore.value.device });
    Object.assign(globalStore.value.device, shown);

    try {
        // Send the change to the device
        const state = await saveSettings(changes);

        // Take the values of the device (the messages come back in capitals, the scene and the holiday may follow each other) unless the user already changed them again
        for (const [field, value] of Object.entries(shown)) {
            const isUnchanged = field === 'messages' ? globalStore.value.device.messages.join() === value.join() : globalStore.value.device[field] === value;
            if (isUnchanged && state[field] !== undefined) {
                globalStore.value.device[field] = state[field];
            }
        }

        // Copy the live fields of the answer
        for (const field of LIVE_FIELDS) {
            if (state[field] !== undefined) globalStore.value.device[field] = state[field];
        }

        // Return that the device took the change
        return true;
    } catch (error) {
        // Tell the user, then show the real settings of the device again
        handleBackendErrors({ error, defaultMessage: translate('common.saveError'), showDialog: true });
        await loadDeviceState().catch(() => {});
        return false;
    }
};

// Save settings while a slider moves: the page changes at once, and the device gets one request when the slider rests
export const createSettingsSaver = (waitMs = 300) => {
    const globalStore = useGlobalStore();
    let pendingChanges = {};

    // Send everything changed since the last request
    const sendPendingChanges = () => {
        const changes = pendingChanges;
        pendingChanges = {};
        return applyDeviceSettings(changes);
    };
    const debouncedSend = createDebounced(sendPendingChanges, waitMs);

    // Show the change and schedule the request
    const save = changes => {
        Object.assign(globalStore.value.device, changes);
        pendingChanges = { ...pendingChanges, ...changes };
        debouncedSend();
    };
    save.flush = debouncedSend.flush;
    return save;
};
