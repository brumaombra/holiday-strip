// Shared device and UI state for the webapp
export const useGlobalStore = () => useState('global', () => ({
    // Cached device state (what GET /api/state answers, with the defaults of the firmware until it is read)
    device: {
        on: true,
        brightness: 60,
        holiday: -1,
        activeHoliday: 0,
        scene: -1,
        maxMilliamps: 3000,
        countdown: true,
        messages: [],
        messageColor: 0,
        language: 'it',
        holidays: [],
        currentScene: 0,
        estimatedMilliamps: 0,
        minMaxMilliamps: 1000,
        maxMaxMilliamps: 8000,
        maxMessageLength: 200
    },

    // Flag to track whether the device state has been fetched at least once
    deviceLoaded: false,

    // Flag set when the device state couldn't be fetched (the pages then offer to try again)
    deviceLoadFailed: false
}));
