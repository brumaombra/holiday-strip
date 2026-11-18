import { afterEach, beforeEach, describe, expect, it, vi } from 'vitest';
import { readBody } from 'h3';
import { flushPromises } from '@vue/test-utils';
import { mountSuspended, registerEndpoint } from '@nuxt/test-utils/runtime';
import HomePage from '~/pages/index.vue';
import MessageEditor from '~/components/message/MessageEditor.vue';
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
const HOLIDAYS = [{
    id: 'christmas',
    name: 'Christmas',
    scenes: [
        { id: 'greeting', name: 'Greeting' },
        { id: 'snow', name: 'Snow' },
        { id: 'trees', name: 'Trees' },
        { id: 'sleigh', name: 'Sleigh' },
        { id: 'fireplace', name: 'Fireplace' },
        { id: 'garland', name: 'Garland' },
        { id: 'gifts', name: 'Gifts' },
        { id: 'candy-cane', name: 'Candy cane' }
    ]
}, {
    id: 'new-year',
    name: 'New Year\'s Eve',
    scenes: [
        { id: 'countdown', name: 'Countdown' },
        { id: 'confetti', name: 'Confetti' },
        { id: 'fireworks', name: 'Fireworks' },
        { id: 'champagne', name: 'Champagne' },
        { id: 'sparklers', name: 'Sparklers' }
    ]
}];
const buildDeviceState = () => ({
    on: true,
    brightness: 128,
    holiday: -1,
    activeHoliday: 0,
    scene: -1,
    maxMilliamps: 3000,
    countdown: true,
    messages: ['BUON NATALE * MERRY CHRISTMAS', 'BUON ANNO * HAPPY NEW YEAR'],
    holidays: HOLIDAYS,
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
            const { messages: changedMessages, ...otherChanges } = body;
            const messages = deviceState.messages.map((message, index) => typeof changedMessages?.[index] === 'string' ? changedMessages[index].toUpperCase() : message);
            deviceState = { ...deviceState, ...otherChanges, messages };

            // Like the firmware: a fixed holiday is the one playing, and a new one goes back to rotating its scenes
            if (body.holiday !== undefined) {
                deviceState.scene = body.scene ?? -1;
                if (body.holiday >= 0) deviceState.activeHoliday = body.holiday;
            }
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
    globalStore.value.device = { ...globalStore.value.device, brightness: 60, on: true, holiday: -1, activeHoliday: 0, scene: -1, messages: [], holidays: [] };
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
        expect(globalStore.value.device.holidays).toEqual(HOLIDAYS);
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
        deviceState = { ...deviceState, brightness: 10, activeHoliday: 1, currentScene: 5, estimatedMilliamps: 777, uptimeSeconds: 99999 };
        await refreshLiveState();
        expect(globalStore.value.device).toMatchObject({ brightness: 200, activeHoliday: 1, currentScene: 5, estimatedMilliamps: 777, uptimeSeconds: 99999 });
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

    // The messages come back in capitals, and a null leaves the message of the other holiday as it is
    it('takes the messages as the device stored them', async () => {
        await loadDeviceState();
        await applyDeviceSettings({ messages: [null, 'ciao anno'] });
        expect(useGlobalStore().value.device.messages).toEqual(['BUON NATALE * MERRY CHRISTMAS', 'CIAO ANNO']);
    });

    // A new holiday plays its own scenes, so the page shows the rotation at once (like the device does)
    it('shows a new holiday with the rotation, and a specific scene pins the holiday', async () => {
        deviceState = { ...deviceState, scene: 3 };
        await loadDeviceState();
        const globalStore = useGlobalStore();
        const pendingChange = applyDeviceSettings({ holiday: 1 });
        expect(globalStore.value.device).toMatchObject({ holiday: 1, activeHoliday: 1, scene: -1 });
        await pendingChange;

        // A scene while Automatic keeps the holiday playing
        deviceState = { ...deviceState, holiday: -1, activeHoliday: 1 };
        await loadDeviceState();
        const pinChange = applyDeviceSettings({ scene: 2 });
        expect(globalStore.value.device).toMatchObject({ holiday: 1, scene: 2 });
        await pinChange;
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

    // The picker cards render in a portal, outside the mounted page
    const sceneCards = () => [...document.querySelectorAll('[data-slot="radio-group-card"]')];

    // The scene row shows the chosen scene and the one on screen, and opens the picker with the rotation and the nine scenes
    it('shows the chosen scene and the one on screen, and lists every scene in the picker', async () => {
        await loadDeviceState();
        const wrapper = await mountPage(HomePage);
        const sceneRow = wrapper.find('[data-testid="scene-row"]');
        expect(sceneRow.text()).toContain('Rotate all scenes');
        expect(sceneRow.text()).toContain('On screen: Christmas trees'); // The third scene (index 2) is on screen

        // Open the picker
        await sceneRow.find('button').trigger('click');
        await flushPromises();
        expect(sceneCards()).toHaveLength(9);
        expect(sceneCards()[0].getAttribute('data-state')).toBe('checked');
        expect(sceneCards()[4].textContent).toContain('Santa\'s sleigh');
    });

    // The scenes are the ones of the holiday playing, translated by id
    it('lists the scenes of the holiday playing', async () => {
        deviceState = { ...deviceState, holiday: 1, activeHoliday: 1, currentScene: 2 };
        await loadDeviceState();
        const wrapper = await mountPage(HomePage);
        expect(wrapper.find('[data-testid="scene-row"]').text()).toContain('On screen: Fireworks');
        await wrapper.find('[data-testid="scene-row"] button').trigger('click');
        await flushPromises();
        expect(sceneCards().map(card => card.textContent)).toEqual([
            expect.stringContaining('Rotate all scenes'),
            expect.stringContaining('Countdown'),
            expect.stringContaining('Confetti'),
            expect.stringContaining('Fireworks'),
            expect.stringContaining('Champagne'),
            expect.stringContaining('Sparklers')
        ]);
    });

    // A scene id the app doesn't know shows the name of the device
    it('falls back to the name of the device for an unknown scene', async () => {
        deviceState = { ...deviceState, holidays: [{ id: 'christmas', name: 'Christmas', scenes: [{ id: 'mystery', name: 'Mystery show' }] }], currentScene: 0 };
        await loadDeviceState();
        const wrapper = await mountPage(HomePage);
        expect(wrapper.find('[data-testid="scene-row"]').text()).toContain('On screen: Mystery show');
    });

    // The holiday row shows Automatic with the holiday the calendar picked
    it('shows the Automatic holiday and the one the calendar picked', async () => {
        await loadDeviceState();
        const wrapper = await mountPage(HomePage);
        const holidayRow = wrapper.find('[data-testid="holiday-row"]');
        expect(holidayRow.text()).toContain('Automatic');
        expect(holidayRow.text()).toContain('Now: Christmas');
    });

    // The picker lists Automatic and one option per holiday
    it('lists Automatic and every holiday in the holiday picker', async () => {
        await loadDeviceState();
        const wrapper = await mountPage(HomePage);
        await wrapper.find('[data-testid="holiday-row"] button').trigger('click');
        await flushPromises();
        expect(sceneCards()).toHaveLength(3);
        expect(sceneCards()[0].getAttribute('data-state')).toBe('checked');
        expect(sceneCards()[0].textContent).toContain('Now: Christmas');
        expect(sceneCards()[1].textContent).toContain('Christmas');
        expect(sceneCards()[2].textContent).toContain('New Year\'s Eve');
    });

    // Picking a holiday sends it, and the scenes follow the new holiday
    it('sends the holiday that is picked, and shows its scenes', async () => {
        await loadDeviceState();
        const wrapper = await mountPage(HomePage);
        await wrapper.find('[data-testid="holiday-row"] button').trigger('click');
        await flushPromises();
        sceneCards()[2].click();
        await vi.waitFor(() => expect(savedBodies).toEqual([{ holiday: 1 }]));
        await vi.waitFor(() => expect(wrapper.find('[data-testid="holiday-row"]').text()).toContain('New Year\'s Eve'));
        expect(useGlobalStore().value.device.activeHoliday).toBe(1);
    });

    // Picking Automatic sends -1
    it('sends -1 to follow the calendar', async () => {
        deviceState = { ...deviceState, holiday: 1, activeHoliday: 1 };
        await loadDeviceState();
        const wrapper = await mountPage(HomePage);
        await wrapper.find('[data-testid="holiday-row"] button').trigger('click');
        await flushPromises();
        sceneCards()[0].click();
        await vi.waitFor(() => expect(savedBodies).toEqual([{ holiday: -1 }]));
    });

    // Picking a scene sends it, and the row shows it as the chosen one
    it('sends the scene that is picked', async () => {
        await loadDeviceState();
        const wrapper = await mountPage(HomePage);
        await wrapper.find('[data-testid="scene-row"] button').trigger('click');
        await flushPromises();
        sceneCards()[5].click();
        await vi.waitFor(() => expect(savedBodies).toEqual([{ scene: 4 }]));
        await vi.waitFor(() => expect(wrapper.find('[data-testid="scene-row"]').text()).toContain('Fireplace')); // Card 5 is scene 4
    });

    // Picking the rotation sends -1
    it('sends -1 to rotate all the scenes', async () => {
        deviceState.scene = 3;
        await loadDeviceState();
        const wrapper = await mountPage(HomePage);
        await wrapper.find('[data-testid="scene-row"] button').trigger('click');
        await flushPromises();
        sceneCards()[0].click();
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
describe('custom message editor', () => {
    // Mount the editor of a holiday, like the dialog of its custom message row does
    const mountEditor = async (holidayIndex = 0) => {
        await loadDeviceState();
        const wrapper = await mountSuspended(MessageEditor, { props: { holidayIndex } });
        mountedWrappers.push(wrapper);
        return wrapper;
    };

    // Type in the message field of a holiday
    const typeMessage = async (wrapper, text, holidayId = 'christmas') => {
        await wrapper.find(`#message-${holidayId}`).setValue(text);
        await flushPromises();
    };

    // The field starts with the message of the device
    it('starts with the message of the device', async () => {
        const wrapper = await mountEditor();
        expect(wrapper.find('#message-christmas').element.value).toBe('BUON NATALE * MERRY CHRISTMAS');
    });

    // The counter follows the typing, against the limit of the device
    it('counts the characters against the limit of the device', async () => {
        const wrapper = await mountEditor();
        await typeMessage(wrapper, 'CIAO');
        expect(wrapper.find('[data-testid="message-counter-christmas"]').text()).toBe('4 / 200');
    });

    // The hint tells what the matrix can draw
    it('lists the characters the matrix can draw', async () => {
        const wrapper = await mountEditor();
        expect(wrapper.text()).toContain('A-Z 0-9');
        expect(wrapper.text()).toContain('! \' * , - . : ?');
        expect(wrapper.text()).toContain('for a heart');
    });

    // A character the font can't draw is flagged before it is sent
    it('warns about characters the font can\'t draw', async () => {
        const wrapper = await mountEditor();
        expect(wrapper.text()).not.toContain('can\'t be drawn');
        await typeMessage(wrapper, 'Buon Natale è bello @');
        expect(wrapper.text()).toContain('Some characters can\'t be drawn');
        expect(wrapper.text()).toContain('è @');
    });

    // A message is only sent with the Save button, null for the other holidays, and the editor then says it's saved
    it('saves only with Save, and only the message of its holiday', async () => {
        const wrapper = await mountEditor(1);
        await typeMessage(wrapper, 'Felice anno nuovo', 'new-year');
        expect(savedBodies).toEqual([]);
        await wrapper.find('form').trigger('submit');
        await vi.waitFor(() => expect(savedBodies).toEqual([{ messages: [null, 'Felice anno nuovo'] }]));

        // The field then shows the message in capitals, like the device stored it, and the dialog is told to close
        await vi.waitFor(() => expect(wrapper.find('#message-new-year').element.value).toBe('FELICE ANNO NUOVO'));
        expect(wrapper.emitted('saved')).toHaveLength(1);
    });

    // Nothing changed, nothing to save
    it('disables Save while the message is unchanged', async () => {
        const wrapper = await mountEditor();
        expect(wrapper.find('[data-testid="message-save-christmas"]').attributes('disabled')).toBeDefined();
        await typeMessage(wrapper, 'NEW');
        expect(wrapper.find('[data-testid="message-save-christmas"]').attributes('disabled')).toBeUndefined();
    });

    // A message that only differs by case is the same for the device
    it('treats a lowercase copy of the message as unchanged', async () => {
        const wrapper = await mountEditor();
        await typeMessage(wrapper, 'buon natale * merry christmas');
        expect(wrapper.find('[data-testid="message-save-christmas"]').attributes('disabled')).toBeDefined();
    });

    // The device refuses more than 200 characters, so the editor doesn't even send it
    it('refuses a message that is too long', async () => {
        const wrapper = await mountEditor();
        await typeMessage(wrapper, 'A'.repeat(201));
        expect(wrapper.find('[data-testid="message-counter-christmas"]').text()).toBe('201 / 200');
        expect(wrapper.find('[data-testid="message-save-christmas"]').attributes('disabled')).toBeDefined();
        await wrapper.find('form').trigger('submit');
        expect(savedBodies).toEqual([]);
    });

    // After a refusal of the device the typed text stays, to be fixed, and the dialog stays open
    it('keeps the typed text when the device refuses it', async () => {
        const wrapper = await mountEditor();
        isSaveRefused = true;
        await typeMessage(wrapper, 'Rifiutato');
        await wrapper.find('form').trigger('submit');
        await vi.waitFor(() => expect(savedBodies).toHaveLength(1));
        await flushPromises();
        expect(wrapper.find('#message-christmas').element.value).toBe('Rifiutato');
        expect(wrapper.emitted('saved')).toBeUndefined();
    });
});

describe('holiday settings on the home page', () => {
    // The options of the open picker dialog
    const pickerOptions = () => [...document.querySelectorAll('[data-slot="radio-group-card"]')];

    // The custom message of the holiday playing sits under the holiday picker, named Custom message
    it('shows the custom message of the holiday playing while Automatic', async () => {
        await loadDeviceState();
        const wrapper = await mountPage(HomePage);
        const row = wrapper.find('[data-testid="message-row-christmas"]');
        expect(row.exists()).toBe(true);
        expect(row.text()).toContain('Custom message');
        expect(row.text()).toContain('BUON NATALE * MERRY CHRISTMAS');
        expect(wrapper.find('[data-testid="message-row-new-year"]').exists()).toBe(false);
    });

    // The settings follow the holiday picked
    it('shows the settings of the holiday picked', async () => {
        deviceState = { ...buildDeviceState(), holiday: 1, activeHoliday: 1 };
        await loadDeviceState();
        const wrapper = await mountPage(HomePage);
        expect(wrapper.find('[data-testid="message-row-new-year"]').text()).toContain('BUON ANNO * HAPPY NEW YEAR');
        expect(wrapper.find('[data-testid="message-row-christmas"]').exists()).toBe(false);
    });

    // The countdown is a Christmas setting: it only shows with Christmas, and applies at once
    it('shows the Christmas countdown only with Christmas, and switches it right away', async () => {
        await loadDeviceState();
        const wrapper = await mountPage(HomePage);
        expect(wrapper.text()).toContain('MANCANO N GIORNI A NATALE!');
        expect(wrapper.text()).toContain('60 days');
        await wrapper.find('#countdown').trigger('click');
        await vi.waitFor(() => expect(savedBodies).toEqual([{ countdown: false }]));

        // Not with New Year's Eve, picked in the holiday picker
        await wrapper.find('[data-testid="holiday-row"] button').trigger('click');
        await flushPromises();
        pickerOptions()[2].click();
        await vi.waitFor(() => expect(wrapper.find('[data-testid="message-row-new-year"]').exists()).toBe(true));
        expect(wrapper.find('#countdown').exists()).toBe(false);
    });

    // The rows sit in one group: the holiday first, then its settings
    it('lists the holiday, then its scene, its custom message and its countdown', async () => {
        await loadDeviceState();
        const wrapper = await mountPage(HomePage);
        const html = wrapper.html();
        expect(html.indexOf('holiday-row')).toBeLessThan(html.indexOf('scene-row'));
        expect(html.indexOf('scene-row')).toBeLessThan(html.indexOf('message-row-christmas'));
        expect(html.indexOf('message-row-christmas')).toBeLessThan(html.indexOf('id="countdown"'));
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
