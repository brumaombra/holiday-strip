import { afterEach, beforeEach, describe, expect, it, vi } from 'vitest';
import { readBody } from 'h3';
import { flushPromises } from '@vue/test-utils';
import { mountSuspended, registerEndpoint } from '@nuxt/test-utils/runtime';
import HomePage from '~/pages/index.vue';
import MessagePage from '~/pages/message.vue';
import SettingsPage from '~/pages/settings.vue';
import { applyDeviceSettings, createSettingsSaver, loadDeviceState, refreshLiveState } from '~/composables/useDeviceState.js';
import { FRAME_BYTES } from '~/composables/useFrame.js';
import { useGlobalStore } from '~/composables/stores/useGlobalStore.js';

// Keep the success toast and the error dialogs out of the page
vi.mock('@brumaombra/ui-vintage/message-toast', () => ({
    showMessageToast: () => {}
}));
vi.mock('@brumaombra/ui-vintage/message-dialog', () => ({
    showMessageDialog: () => {}
}));

// State of the device, as GET /api/state answers it
const SCENES = ['Greeting', 'Snowfall', 'Christmas trees', 'Santa\'s sleigh', 'Fireplace', 'Fairy lights', 'Gifts', 'Candy cane', 'Fireworks'];
const buildDeviceState = () => ({
    on: true,
    brightness: 128,
    scene: -1,
    maxMilliamps: 3000,
    countdown: true,
    message: 'BUON NATALE * MERRY CHRISTMAS',
    scenes: SCENES,
    currentScene: 2,
    estimatedMilliamps: 1200,
    minMaxMilliamps: 1000,
    maxMaxMilliamps: 8000,
    maxMessageLength: 200,
    uptimeSeconds: 93784,
    ssid: 'Home'
});

// Details of the device, as GET /api/device answers them (the device card of the gizmo-common layer reads them)
const buildDeviceDetails = () => ({
    name: 'Holiday Strip',
    version: 'v2026.11.04.1',
    uptimeSeconds: 93784,
    freeHeap: 187432,
    ssid: 'Home',
    accessPointActive: false,
    ip: '192.168.1.42',
    hostname: 'holiday-strip.local',
    signalQuality: 74
});

// The device API: its state, its details, whether it answers, the bodies of the saves and the frames asked
let deviceDetails;
let deviceState;
let isDeviceAvailable;
let isSaveRefused;
let savedBodies;
let frameCount;

// Answer like the device, or fail like a device that doesn't answer
const registerDeviceEndpoints = () => {
    registerEndpoint('/api/state', () => {
        if (!isDeviceAvailable) throw createError({ statusCode: 500, data: { errorId: null, errorMessage: 'Unreachable', data: null } });
        return { errorId: null, errorMessage: null, data: deviceState };
    });
    registerEndpoint('/api/settings', {
        method: 'POST',
        handler: async event => {
            const body = await readBody(event);
            savedBodies.push(body);
            if (isSaveRefused) throw createError({ statusCode: 400, data: { errorId: 'invalid_settings_field', errorMessage: Object.keys(body)[0], data: null } });
            deviceState = { ...deviceState, ...body, message: typeof body.message === 'string' ? body.message.toUpperCase() : deviceState.message };
            return { errorId: null, errorMessage: null, data: deviceState };
        }
    });
    registerEndpoint('/api/device', () => ({ errorId: null, errorMessage: null, data: deviceDetails }));
    registerEndpoint('/api/frame', () => {
        frameCount += 1;
        return new Uint8Array(FRAME_BYTES);
    });
};

// Pages mounted by the test, unmounted afterwards (a page left mounted keeps reading frames)
const mountedWrappers = [];

// Mount a page and remember it
const mountPage = async page => {
    const wrapper = await mountSuspended(page);
    mountedWrappers.push(wrapper);
    return wrapper;
};

// Find a button by its text
const findButton = (wrapper, text) => wrapper.findAll('button').find(button => button.text().includes(text));

// Start each test with the store of a page that has not loaded the state yet
beforeEach(() => {
    deviceState = buildDeviceState();
    deviceDetails = buildDeviceDetails();
    isDeviceAvailable = true;
    isSaveRefused = false;
    savedBodies = [];
    frameCount = 0;
    const globalStore = useGlobalStore();
    globalStore.value.deviceLoaded = false;
    globalStore.value.deviceLoadFailed = false;
    globalStore.value.device = { ...globalStore.value.device, brightness: 60, on: true, scene: -1, message: '', scenes: [] };
    vi.spyOn(console, 'error').mockImplementation(() => {});
    registerDeviceEndpoints();
});

// Unmount the pages and close the dialogs between tests
afterEach(() => {
    while (mountedWrappers.length) mountedWrappers.pop().unmount();
    document.body.innerHTML = '';
});

// Loading the state: the pages may only show and save what the device sent
describe('loadDeviceState', () => {
    // A loaded state replaces the store defaults
    it('loads the state of the device', async () => {
        await loadDeviceState();
        const globalStore = useGlobalStore();
        expect(globalStore.value.deviceLoaded).toBe(true);
        expect(globalStore.value.device.brightness).toBe(128);
        expect(globalStore.value.device.scenes).toEqual(SCENES);
    });

    // A failed load is remembered, and the store keeps its defaults marked as not loaded
    it('remembers a failed load', async () => {
        isDeviceAvailable = false;
        await expect(loadDeviceState()).rejects.toThrow();
        const globalStore = useGlobalStore();
        expect(globalStore.value.deviceLoaded).toBe(false);
        expect(globalStore.value.deviceLoadFailed).toBe(true);
    });

    // The live refresh used to replace the whole state, which undid a slider the user was moving
    it('refreshes only what the device changes by itself', async () => {
        await loadDeviceState();
        const globalStore = useGlobalStore();
        globalStore.value.device.brightness = 200;
        deviceState = { ...deviceState, brightness: 10, currentScene: 5, estimatedMilliamps: 777, uptimeSeconds: 99999 };
        await refreshLiveState();
        expect(globalStore.value.device).toMatchObject({ brightness: 200, currentScene: 5, estimatedMilliamps: 777, uptimeSeconds: 99999 });
    });
});

// Applying settings: shown at once, confirmed by the device, undone when it refuses
describe('applyDeviceSettings', () => {
    // The page doesn't wait for the device to show a switch flipped
    it('sends the change and keeps it', async () => {
        await loadDeviceState();
        const isApplied = await applyDeviceSettings({ on: false });
        expect(isApplied).toBe(true);
        expect(savedBodies).toEqual([{ on: false }]);
        expect(useGlobalStore().value.device.on).toBe(false);
    });

    // The message comes back in capitals
    it('takes the message as the device stored it', async () => {
        await loadDeviceState();
        await applyDeviceSettings({ message: 'ciao natale' });
        expect(useGlobalStore().value.device.message).toBe('CIAO NATALE');
    });

    // A refused change used to stay on screen while the device kept the old value
    it('brings back the real settings when the device refuses', async () => {
        await loadDeviceState();
        isSaveRefused = true;
        const isApplied = await applyDeviceSettings({ brightness: 999 });
        expect(isApplied).toBe(false);
        await vi.waitFor(() => expect(useGlobalStore().value.device.brightness).toBe(128));
    });

    // A slider drag sends one request, with the last value
    it('sends one request when a slider rests', async () => {
        vi.useFakeTimers();
        await loadDeviceState();
        const save = createSettingsSaver(300);
        save({ brightness: 10 });
        save({ brightness: 20 });
        save({ brightness: 30 });
        expect(useGlobalStore().value.device.brightness).toBe(30);
        expect(savedBodies).toEqual([]);
        await vi.advanceTimersByTimeAsync(300);
        vi.useRealTimers();
        await vi.waitFor(() => expect(savedBodies).toEqual([{ brightness: 30 }]));
    });

    // Leaving the page right after a drag used to lose the change
    it('sends a waiting change when flushed', async () => {
        await loadDeviceState();
        const save = createSettingsSaver(60000);
        save({ maxMilliamps: 5000 });
        save.flush();
        await vi.waitFor(() => expect(savedBodies).toEqual([{ maxMilliamps: 5000 }]));
    });
});

// Home page: preview, switch, brightness and scenes
describe('home page', () => {
    // While the state is loading there is no control that could send defaults
    it('waits for the state before showing the controls', async () => {
        const wrapper = await mountPage(HomePage);
        expect(wrapper.find('canvas').exists()).toBe(false);
        expect(wrapper.text()).toContain('Syncing device');
    });

    // A device that doesn't answer gets a retry, not a page of defaults
    it('offers to try again when the state failed to load', async () => {
        isDeviceAvailable = false;
        await loadDeviceState().catch(() => {});
        const wrapper = await mountPage(HomePage);
        expect(wrapper.text()).toContain('Couldn\'t read the device');
        expect(wrapper.find('canvas').exists()).toBe(false);

        // Try again once the device answers
        isDeviceAvailable = true;
        await findButton(wrapper, 'Try again').trigger('click');
        await vi.waitFor(() => expect(wrapper.find('canvas').exists()).toBe(true));
    });

    // The preview draws the frames of the device on a canvas
    it('shows the live preview canvas and reads frames', async () => {
        await loadDeviceState();
        const wrapper = await mountPage(HomePage);
        const canvas = wrapper.find('canvas');
        expect(canvas.exists()).toBe(true);
        expect(canvas.attributes('width')).toBe('864');
        expect(canvas.attributes('height')).toBe('144');
        await vi.waitFor(() => expect(frameCount).toBeGreaterThan(0));
    });

    // Leaving the page must stop reading frames
    it('stops reading frames when the page is left', async () => {
        await loadDeviceState();
        const wrapper = await mountPage(HomePage);
        await vi.waitFor(() => expect(frameCount).toBeGreaterThan(0));
        wrapper.unmount();
        await flushPromises();
        const framesAtUnmount = frameCount;
        await new Promise(resolve => setTimeout(resolve, 600));
        expect(frameCount).toBeLessThanOrEqual(framesAtUnmount + 1);
    });

    // The rotation and the nine scenes are there, with the scene on screen marked
    it('lists the rotation and every scene, and marks the one on screen', async () => {
        await loadDeviceState();
        const wrapper = await mountPage(HomePage);
        const sceneRows = wrapper.findAll('[data-testid="scene-row"]');
        expect(sceneRows).toHaveLength(10);
        expect(sceneRows[0].text()).toContain('Rotate all scenes');
        expect(sceneRows[0].find('[data-testid="scene-selected"]').exists()).toBe(true);
        expect(sceneRows[4].text()).toContain('Santa\'s sleigh');

        // The third scene (index 2) is on screen
        const onScreen = sceneRows.filter(row => row.text().includes('On screen'));
        expect(onScreen).toHaveLength(1);
        expect(onScreen[0].text()).toContain('Christmas trees');
    });

    // Picking a scene sends it, and it becomes the chosen one
    it('sends the scene that is picked', async () => {
        await loadDeviceState();
        const wrapper = await mountPage(HomePage);
        await wrapper.findAll('[data-testid="scene-row"]')[5].find('button').trigger('click');
        await vi.waitFor(() => expect(savedBodies).toEqual([{ scene: 4 }]));
        expect(wrapper.findAll('[data-testid="scene-row"]')[5].find('[data-testid="scene-selected"]').exists()).toBe(true);
    });

    // Picking the rotation sends -1
    it('sends -1 to rotate all the scenes', async () => {
        deviceState.scene = 3;
        await loadDeviceState();
        const wrapper = await mountPage(HomePage);
        await wrapper.findAll('[data-testid="scene-row"]')[0].find('button').trigger('click');
        await vi.waitFor(() => expect(savedBodies).toEqual([{ scene: -1 }]));
    });

    // The power switch applies at once
    it('switches the display off right away', async () => {
        await loadDeviceState();
        const wrapper = await mountPage(HomePage);
        await wrapper.find('#power').trigger('click');
        await vi.waitFor(() => expect(savedBodies).toEqual([{ on: false }]));
    });

    // The estimated current is shown against the limit
    it('shows the estimated current against the limit', async () => {
        await loadDeviceState();
        const wrapper = await mountPage(HomePage);
        expect(wrapper.text()).toContain('1200 / 3000 mA');
        expect(wrapper.text()).toContain('Limit 3000 mA');
        expect(wrapper.text()).not.toContain('Power limit reached');
    });

    // At the limit the device dims the LEDs, and the page says so
    it('warns when the current reaches the limit', async () => {
        deviceState.estimatedMilliamps = 3000;
        await loadDeviceState();
        const wrapper = await mountPage(HomePage);
        expect(wrapper.text()).toContain('Power limit reached');
    });
});

// Message page: explicit save, a counter and a warning
describe('message page', () => {
    // Type in the message field
    const typeMessage = async (wrapper, text) => {
        await wrapper.find('#message').setValue(text);
        await flushPromises();
    };

    // The field starts with the message of the device
    it('starts with the message of the device', async () => {
        await loadDeviceState();
        const wrapper = await mountPage(MessagePage);
        expect(wrapper.find('#message').element.value).toBe('BUON NATALE * MERRY CHRISTMAS');
    });

    // The counter follows the typing, against the limit of the device
    it('counts the characters against the limit of the device', async () => {
        await loadDeviceState();
        const wrapper = await mountPage(MessagePage);
        await typeMessage(wrapper, 'CIAO');
        expect(wrapper.find('[data-testid="message-counter"]').text()).toBe('4 / 200');
    });

    // The hint tells what the matrix can draw
    it('lists the characters the matrix can draw', async () => {
        await loadDeviceState();
        const wrapper = await mountPage(MessagePage);
        expect(wrapper.text()).toContain('A-Z 0-9');
        expect(wrapper.text()).toContain('! \' * , - . : ?');
        expect(wrapper.text()).toContain('for a heart');
    });

    // A character the font can't draw is flagged before it is sent
    it('warns about characters the font can\'t draw', async () => {
        await loadDeviceState();
        const wrapper = await mountPage(MessagePage);
        expect(wrapper.text()).not.toContain('can\'t be drawn');
        await typeMessage(wrapper, 'Buon Natale è bello @');
        expect(wrapper.text()).toContain('Some characters can\'t be drawn');
        expect(wrapper.text()).toContain('è @');
    });

    // A message is only sent with the Save button
    it('saves nothing until Save is pressed', async () => {
        await loadDeviceState();
        const wrapper = await mountPage(MessagePage);
        await typeMessage(wrapper, 'Ciao Natale');
        expect(savedBodies).toEqual([]);
        await wrapper.find('form').trigger('submit');
        await vi.waitFor(() => expect(savedBodies).toEqual([{ message: 'Ciao Natale' }]));

        // The field then shows the message in capitals, like the device stored it
        await vi.waitFor(() => expect(wrapper.find('#message').element.value).toBe('CIAO NATALE'));
    });

    // Nothing changed, nothing to save
    it('disables Save while the message is unchanged', async () => {
        await loadDeviceState();
        const wrapper = await mountPage(MessagePage);
        expect(findButton(wrapper, 'Save message').attributes('disabled')).toBeDefined();
        await typeMessage(wrapper, 'NEW');
        expect(findButton(wrapper, 'Save message').attributes('disabled')).toBeUndefined();
    });

    // A message that only differs by case is the same for the device
    it('treats a lowercase copy of the message as unchanged', async () => {
        await loadDeviceState();
        const wrapper = await mountPage(MessagePage);
        await typeMessage(wrapper, 'buon natale * merry christmas');
        expect(findButton(wrapper, 'Save message').attributes('disabled')).toBeDefined();
    });

    // The device refuses more than 200 characters, so the page doesn't even send it
    it('refuses a message that is too long', async () => {
        await loadDeviceState();
        const wrapper = await mountPage(MessagePage);
        await typeMessage(wrapper, 'A'.repeat(201));
        expect(wrapper.find('[data-testid="message-counter"]').text()).toBe('201 / 200');
        expect(findButton(wrapper, 'Save message').attributes('disabled')).toBeDefined();
        await wrapper.find('form').trigger('submit');
        expect(savedBodies).toEqual([]);
    });

    // After a refusal of the device the typed text stays, to be fixed
    it('keeps the typed text when the device refuses it', async () => {
        await loadDeviceState();
        const wrapper = await mountPage(MessagePage);
        isSaveRefused = true;
        await typeMessage(wrapper, 'Rifiutato');
        await wrapper.find('form').trigger('submit');
        await vi.waitFor(() => expect(savedBodies).toHaveLength(1));
        await flushPromises();
        expect(wrapper.find('#message').element.value).toBe('Rifiutato');
    });

    // The countdown switch applies at once
    it('turns the countdown off right away, and explains it', async () => {
        await loadDeviceState();
        const wrapper = await mountPage(MessagePage);
        expect(wrapper.text()).toContain('MANCANO N GIORNI A NATALE!');
        expect(wrapper.text()).toContain('60 days');
        await wrapper.find('#countdown').trigger('click');
        await vi.waitFor(() => expect(savedBodies).toEqual([{ countdown: false }]));
    });
});

// Settings page: power limit, language and device info
describe('settings page', () => {
    // The slider covers the range the device announces, in steps of 100
    it('limits the power slider to the range of the device', async () => {
        await loadDeviceState();
        const wrapper = await mountPage(SettingsPage);
        const slider = wrapper.find('[role="slider"]');
        expect(slider.attributes('aria-valuemin')).toBe('1000');
        expect(slider.attributes('aria-valuemax')).toBe('8000');
        expect(wrapper.text()).toContain('3000mA');
        expect(wrapper.text()).toContain('80%');
    });

    // The slider sends one request when it rests
    it('saves the power limit once the slider rests', async () => {
        await loadDeviceState();
        const wrapper = await mountPage(SettingsPage);
        await wrapper.find('[role="slider"]').trigger('keydown', { key: 'ArrowRight' });
        await vi.waitFor(() => expect(savedBodies).toEqual([{ maxMilliamps: 3100 }]), { timeout: 3000 });
    });

    // The device rows come from the card of the gizmo-common layer, which reads GET /api/device
    it('shows the device card with the details of the device', async () => {
        await loadDeviceState();
        const wrapper = await mountPage(SettingsPage);
        await vi.waitFor(() => expect(wrapper.text()).toContain('v2026.11.04.1'));
        expect(wrapper.text()).toContain('1d 2h 3m');
        expect(wrapper.text()).toContain('Home');
        expect(wrapper.text()).toContain('192.168.1.42');
        expect(findButton(wrapper, 'Restart')).toBeDefined();
        expect(findButton(wrapper, 'Forget network')).toBeDefined();
    });

    // Restarting is most needed when the settings can't be read, so the card doesn't wait for them
    it('shows the device card even when the settings cannot be read', async () => {
        isDeviceAvailable = false;
        await expect(loadDeviceState()).rejects.toThrow();
        const wrapper = await mountPage(SettingsPage);
        await vi.waitFor(() => expect(findButton(wrapper, 'Restart')).toBeDefined());
        expect(wrapper.text()).toContain('Try again');
    });

    // The language row opens a picker with both languages
    it('offers both languages', async () => {
        await loadDeviceState();
        const wrapper = await mountPage(SettingsPage);
        expect(wrapper.text()).toContain('English');
        await findButton(wrapper, 'English').trigger('click');
        await vi.waitFor(() => expect(document.body.textContent).toContain('Italiano'));
    });
});
