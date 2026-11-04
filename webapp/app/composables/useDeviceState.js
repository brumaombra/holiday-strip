import { getState, saveSettings } from '~/composables/useDeviceApi.js';
import { createDebounced, handleBackendErrors, translate } from '~/composables/useUtils.js';
import { useGlobalStore } from '~/composables/stores/useGlobalStore.js';

// Fields the device changes by itself, and the page only reads
const LIVE_FIELDS = ['currentScene', 'estimatedMilliamps', 'uptimeSeconds'];

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

// Refresh only what the device changes by itself (the scene on screen, the estimated current, the uptime), so a setting being edited is never overwritten
export const refreshLiveState = async () => {
    const globalStore = useGlobalStore();
    const state = await getState();

    // Copy the live fields into the store
    for (const field of LIVE_FIELDS) {
        if (state[field] !== undefined) globalStore.value.device[field] = state[field];
    }
};

// Change some settings: the page shows them right away, the device confirms them, and a refusal brings back what the device really has
export const applyDeviceSettings = async changes => {
    const globalStore = useGlobalStore();

    // Show the change right away
    Object.assign(globalStore.value.device, changes);

    try {
        // Send the change to the device
        const state = await saveSettings(changes);

        // Take the values of the device (the message comes back in capitals) unless the user already changed them again
        for (const [field, value] of Object.entries(changes)) {
            if (globalStore.value.device[field] === value && state[field] !== undefined) {
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
