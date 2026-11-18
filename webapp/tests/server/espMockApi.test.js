import { IncomingMessage, ServerResponse } from 'node:http';
import { Socket } from 'node:net';
import { afterEach, beforeEach, describe, expect, it, vi } from 'vitest';
import { createEvent } from 'h3';

// Stub the Nitro globals the routes of this app use before the handler modules load (the mock of the gizmo-common layer imports its h3 functions, so it runs on real h3 events)
vi.hoisted(() => {
    globalThis.defineEventHandler = handler => handler;
    globalThis.defineNitroPlugin = plugin => plugin;
    globalThis.useAppConfig = () => ({ gizmo: { name: 'Holiday Strip' } });
    globalThis.setResponseHeader = (event, name, value) => {
        event.node.res.setHeader(name, value);
    };
});

// Load the handlers of the mock device API
const { default: getState } = await import('../../server/api/state/index.get.js');
const { default: postSettings } = await import('../../server/api/settings/index.post.js');
const { default: getFrame } = await import('../../server/api/frame/index.get.js');
const { HOLIDAYS, getEspMockState } = await import('../../server/utils/esp-mock-state.js');
const { default: nameMockDevice } = await import('../../server/plugins/gizmo-mock.js');
const { buildDevicePayload, resetGizmoMockState } = await import('gizmo-common/web/server/utils/gizmo-mock.js');

// Build a real h3 request event, with a JSON body when one is given (a string is sent as it is)
const buildEvent = (body = undefined) => {
    const request = new IncomingMessage(new Socket());
    request.method = 'POST';
    request.rawBody = Buffer.from(body === undefined ? '' : (typeof body === 'string' ? body : JSON.stringify(body)));
    return createEvent(request, new ServerResponse(request));
};

// Call a handler and return its answer with the status code it set
const callHandler = async (handler, event = buildEvent()) => {
    const response = await handler(event);
    return { statusCode: event.node.res.statusCode, response, event };
};

// Start every test from the demo device the dev server boots with, on a day when the calendar picks Christmas (so the tests never depend on the real date)
beforeEach(() => {
    resetGizmoMockState();
    vi.useFakeTimers({ toFake: ['Date'] });
    vi.setSystemTime(new Date('2026-11-18T12:00:00'));
});

// Give back the real clock
afterEach(() => {
    vi.useRealTimers();
});

// Response shape: the web app reads the same { errorId, errorMessage, data } as on the device
describe('mock device responses', () => {
    // A success has no error and the data
    it('answers a success like the firmware', async () => {
        const { statusCode, response } = await callHandler(getState);
        expect(statusCode).toBe(200);
        expect(response.errorId).toBeNull();
        expect(response.errorMessage).toBeNull();
        expect(response.data.brightness).toBe(60);
    });

    // The mock used to send h3 errors, so the app never found the errorId and never showed its translated device errors
    it('answers an error as the firmware body with its status code', async () => {
        const { statusCode, response } = await callHandler(postSettings, buildEvent({ brightness: 256 }));
        expect(statusCode).toBe(400);
        expect(response).toEqual({ errorId: 'invalid_settings_field', errorMessage: 'brightness', data: null });
    });

    // Bodies are checked like gizmo-common reads them
    it('refuses invalid, non-object and oversized bodies', async () => {
        const invalidJson = await callHandler(postSettings, buildEvent('{nope'));
        const arrayBody = await callHandler(postSettings, buildEvent([1]));
        const largeBody = await callHandler(postSettings, buildEvent({ text: 'a'.repeat(2100) }));
        expect([invalidJson.statusCode, invalidJson.response.errorMessage]).toEqual([400, 'Invalid JSON']);
        expect([arrayBody.statusCode, arrayBody.response.errorMessage]).toEqual([400, 'Invalid JSON object']);
        expect([largeBody.statusCode, largeBody.response.errorId]).toEqual([413, 'request_body_too_large']);
    });
});

// GET /api/state: the settings and what the device knows about itself
describe('mock state', () => {
    // The dashboard needs all of these fields to draw its pages
    it('answers the firmware defaults and the device info', async () => {
        const { response } = await callHandler(getState);
        expect(response.data).toMatchObject({
            on: true,
            brightness: 60,
            holiday: -1,
            scene: -1,
            maxMilliamps: 3000,
            messages: ['BUON NATALE!', 'BUON ANNO!', 'CIAO!'],
            messageColor: 0,
            language: 'it',
            holidays: HOLIDAYS,
            minMaxMilliamps: 1000,
            maxMaxMilliamps: 8000,
            maxMessageLength: 200
        });
        expect(response.data.holidays.map(holiday => holiday.id)).toEqual(['christmas', 'new-year', 'custom']);
        expect(response.data.holidays.map(holiday => holiday.scenes.length)).toEqual([9, 5, 1]);
        expect(response.data.scenes).toBeUndefined();
        expect(response.data.message).toBeUndefined();
        expect(response.data.currentScene).toBeGreaterThanOrEqual(0);
        expect(response.data.currentScene).toBeLessThan(response.data.holidays[response.data.activeHoliday].scenes.length);
        expect(response.data.uptimeSeconds).toBeUndefined(); // The device info is GET /api/device, read by the device card
        expect(response.data.ssid).toBeUndefined();
    });

    // A chosen scene is the one on screen, while the rotation moves on by itself
    it('shows the chosen scene as the current one', async () => {
        await callHandler(postSettings, buildEvent({ holiday: 0, scene: 4 }));
        const { response } = await callHandler(getState);
        expect(response.data.currentScene).toBe(4);
    });

    // Automatic follows the calendar: New Year's Eve from December 27 to January 1 included, Christmas the rest of the year
    it.each([
        ['2026-11-18T12:00:00', 0],
        ['2026-12-26T12:00:00', 0],
        ['2026-12-27T12:00:00', 1],
        ['2026-12-31T23:30:00', 1],
        ['2027-01-01T12:00:00', 1],
        ['2027-01-02T12:00:00', 0]
    ])('resolves Automatic on %s to holiday %i', async (date, expectedHoliday) => {
        vi.useFakeTimers();
        vi.setSystemTime(new Date(date));
        const { response } = await callHandler(getState);
        vi.useRealTimers();
        expect(response.data.holiday).toBe(-1);
        expect(response.data.activeHoliday).toBe(expectedHoliday);
    });

    // A chosen holiday wins over the calendar, and the rotation stays inside its scenes
    it('plays the chosen holiday and rotates only its scenes', async () => {
        vi.useFakeTimers();
        vi.setSystemTime(new Date('2026-11-18T12:00:00'));
        await callHandler(postSettings, buildEvent({ holiday: 1 }));
        const scenes = new Set();
        for (let step = 0; step < 12; step += 1) {
            vi.advanceTimersByTime(15000);
            scenes.add((await callHandler(getState)).response.data.currentScene);
        }
        const { response } = await callHandler(getState);
        vi.useRealTimers();
        expect(response.data.activeHoliday).toBe(1);
        expect([...scenes].every(scene => scene >= 0 && scene < 5)).toBe(true);
        expect(scenes.size).toBe(5);
    });

    // A dark display draws no current
    it('estimates no current while the display is off', async () => {
        await callHandler(postSettings, buildEvent({ on: false }));
        const { response } = await callHandler(getState);
        expect(response.data.estimatedMilliamps).toBe(0);
    });

    // The limiter keeps the estimate under the power limit
    it('never estimates more than the power limit', async () => {
        await callHandler(postSettings, buildEvent({ brightness: 255, maxMilliamps: 1000 }));
        const { response } = await callHandler(getState);
        expect(response.data.estimatedMilliamps).toBeLessThanOrEqual(1000);
    });
});

// POST /api/settings: the validation of the firmware
describe('mock settings validation', () => {
    // Send a body and return the answer
    const send = body => callHandler(postSettings, buildEvent(body));

    // The answer is the state after the change, so the app needs no second request
    it('answers the state after the change', async () => {
        const { statusCode, response } = await send({ brightness: 120, holiday: 0, scene: 2, on: false });
        expect(statusCode).toBe(200);
        expect(response.data).toMatchObject({ brightness: 120, holiday: 0, scene: 2, on: false, holidays: HOLIDAYS });
    });

    // Any subset of the fields is accepted, and the others keep their value
    it('changes only the fields that are sent', async () => {
        await send({ on: false });
        const { response } = await send({});
        expect(response.data).toMatchObject({ on: false, brightness: 60, maxMilliamps: 3000 });
    });

    // The device stores the messages in capitals
    it('stores the messages in capitals', async () => {
        const { response } = await send({ messages: ['Merry Christmas <3', 'Happy new year'] });
        expect(response.data.messages).toEqual(['MERRY CHRISTMAS <3', 'HAPPY NEW YEAR', 'CIAO!']);
    });

    // A null leaves the message of that holiday as it is
    it('leaves the message of a holiday that gets null', async () => {
        const { response } = await send({ messages: [null, 'Happy new year'] });
        expect(response.data.messages).toEqual(['BUON NATALE!', 'HAPPY NEW YEAR', 'CIAO!']);
    });

    // The list may be shorter than the holidays: the missing ones are not touched
    it('accepts a messages list shorter than the holidays', async () => {
        const { statusCode, response } = await send({ messages: ['Ciao'] });
        expect(statusCode).toBe(200);
        expect(response.data.messages).toEqual(['CIAO', 'BUON ANNO!', 'CIAO!']);
    });

    // The custom message is picked like a holiday, with its own message and color, and the calendar never picks it
    it('plays the custom message with its message and color', async () => {
        const { statusCode, response } = await send({ holiday: 2, messages: [null, null, 'saldi!'], messageColor: 6 });
        expect(statusCode).toBe(200);
        expect(response.data).toMatchObject({ holiday: 2, activeHoliday: 2, scene: -1, messageColor: 6 });
        expect(response.data.messages).toEqual(['BUON NATALE!', 'BUON ANNO!', 'SALDI!']);
    });

    // The language translates the greetings still at their default, and leaves the ones written by hand
    it('translates the default greetings when the language changes', async () => {
        await send({ messages: [null, 'Auguri a tutti'] });
        const { statusCode, response } = await send({ language: 'en' });
        expect(statusCode).toBe(200);
        expect(response.data.language).toBe('en');
        expect(response.data.messages).toEqual(['MERRY CHRISTMAS!', 'AUGURI A TUTTI', 'HELLO!']);
    });

    // An unknown language is refused
    it('refuses a language it does not have', async () => {
        const { statusCode, response } = await send({ language: 'fr' });
        expect(statusCode).toBe(400);
        expect(response.errorMessage).toBe('language');
    });

    // Changing the holiday without a scene goes back to rotating
    it('resets the scene to rotate when the holiday changes', async () => {
        await send({ holiday: 0, scene: 3 });
        const { response } = await send({ holiday: 1 });
        expect(response.data).toMatchObject({ holiday: 1, activeHoliday: 1, scene: -1 });
    });

    // Sending the holiday that is already chosen keeps the chosen scene, like the firmware
    it('keeps the scene when the same holiday is sent again', async () => {
        await send({ holiday: 0, scene: 3 });
        const { response } = await send({ holiday: 0 });
        expect(response.data).toMatchObject({ holiday: 0, scene: 3 });
    });

    // A scene sent together with a new holiday is kept
    it('keeps the scene sent together with a new holiday', async () => {
        const { response } = await send({ holiday: 1, scene: 4 });
        expect(response.data).toMatchObject({ holiday: 1, scene: 4, currentScene: 4 });
    });

    // A specific scene only makes sense inside a fixed holiday
    it('pins the holiday playing when a scene is sent while Automatic', async () => {
        vi.useFakeTimers();
        vi.setSystemTime(new Date('2026-12-30T12:00:00'));
        const { response } = await send({ scene: 2 });
        vi.useRealTimers();
        expect(response.data).toMatchObject({ holiday: 1, activeHoliday: 1, scene: 2 });
    });

    // Rotating (-1) is not a specific scene, so Automatic stays
    it('keeps Automatic when the scene sent is -1', async () => {
        const { response } = await send({ scene: -1 });
        expect(response.data.holiday).toBe(-1);
    });

    // The scene is checked against the holiday it ends up in
    it('validates the scene against the scenes of the resulting holiday', async () => {
        // Scene 6 exists in Christmas (8 scenes) but not in New Year's Eve (5)
        const christmas = await send({ holiday: 0, scene: 6 });
        const newYear = await send({ holiday: 1, scene: 6 });
        expect(christmas.statusCode).toBe(200);
        expect(newYear.response).toEqual({ errorId: 'invalid_settings_field', errorMessage: 'scene', data: null });

        // The holiday already chosen counts when none is sent
        await send({ holiday: 1 });
        const sameHoliday = await send({ scene: 6 });
        expect(sameHoliday.statusCode).toBe(400);
        expect((await send({ scene: 4 })).statusCode).toBe(200);
    });

    // While Automatic, the scenes are the ones of the holiday the calendar picked
    it('validates the scene against the holiday of the calendar while Automatic', async () => {
        vi.useFakeTimers();
        vi.setSystemTime(new Date('2026-12-30T12:00:00'));
        const refused = await send({ scene: 5 });
        const accepted = await send({ scene: 4 });
        vi.useRealTimers();
        expect(refused.response.errorMessage).toBe('scene');
        expect(accepted.statusCode).toBe(200);
    });

    // The mock used to save any field it got, while the firmware ignores the unknown ones
    it('ignores the fields it does not know', async () => {
        const { response } = await send({ ssid: 'elsewhere', uptimeSeconds: 99999, scenes: [], message: 'OLD', activeHoliday: 1, currentScene: 8 });
        expect(response.data.ssid).toBeUndefined();
        expect(response.data.scenes).toBeUndefined();
        expect(response.data.messages[0]).toBe('BUON NATALE!');
        expect(response.data.activeHoliday).toBe(0);
        expect(response.data.uptimeSeconds).toBeUndefined();
    });

    // Every field refuses the wrong type, with its name in the error
    it.each([
        ['on', 1],
        ['on', 'true'],
        ['brightness', '60'],
        ['brightness', 12.5],
        ['holiday', '1'],
        ['holiday', 0.5],
        ['scene', '1'],
        ['scene', 1.5],
        ['maxMilliamps', '3000'],
        ['maxMilliamps', 2000.5],
        ['messages', 'hello'],
        ['messages', [42]],
        ['messages', [null, {}]]
    ])('refuses %s = %j (wrong type)', async (field, value) => {
        const { statusCode, response } = await send({ [field]: value });
        expect(statusCode).toBe(400);
        expect(response).toEqual({ errorId: 'invalid_settings_field', errorMessage: field, data: null });
    });

    // Every field refuses a value out of range
    it.each([
        ['brightness', -1],
        ['brightness', 256],
        ['holiday', -2],
        ['holiday', 3],
        ['messageColor', -1],
        ['messageColor', 7],
        ['scene', -2],
        ['scene', 9],
        ['maxMilliamps', 999],
        ['maxMilliamps', 8001]
    ])('refuses %s = %j (out of range)', async (field, value) => {
        const { statusCode, response } = await send({ [field]: value });
        expect(statusCode).toBe(400);
        expect(response.errorMessage).toBe(field);
    });

    // The edges of the ranges are valid
    it.each([
        ['brightness', 0],
        ['brightness', 255],
        ['holiday', -1],
        ['holiday', 1],
        ['scene', -1],
        ['scene', 7],
        ['maxMilliamps', 1000],
        ['maxMilliamps', 8000]
    ])('accepts %s = %j (edge of the range)', async (field, value) => {
        const { statusCode, response } = await send({ [field]: value });
        expect(statusCode).toBe(200);
        expect(response.data[field]).toBe(value);
    });

    // The message is limited to 200 bytes
    it('refuses a message longer than 200 bytes, and accepts exactly 200', async () => {
        const exact = await send({ messages: [null, 'A'.repeat(200)] });
        const tooLong = await send({ messages: ['A'.repeat(201)] });
        expect(exact.statusCode).toBe(200);
        expect(tooLong.response).toEqual({ errorId: 'invalid_settings_field', errorMessage: 'messages', data: null });
    });

    // One message per holiday, no more
    it('refuses more messages than holidays', async () => {
        const { response } = await send({ messages: ['A', 'B', 'C', 'D'] });
        expect(response.errorMessage).toBe('messages');
    });

    // The device counts bytes (strlen), so accented letters weigh two
    it('counts the message in bytes', async () => {
        const { statusCode } = await send({ messages: ['È'.repeat(101)] });
        expect(statusCode).toBe(400);
    });

    // A refused body changes nothing, even the valid fields in it
    it('applies nothing when one field is wrong', async () => {
        await send({ brightness: 10, scene: 99, messages: ['CHANGED'] });
        const { response } = await callHandler(getState);
        expect(response.data.brightness).toBe(60);
        expect(getEspMockState().settings.scene).toBe(-1);
        expect(getEspMockState().settings.messages[0]).toBe('BUON NATALE!');
    });

    // Several wrong fields report the first one in the firmware order (on, brightness, holiday, scene, maxMilliamps, messages)
    it('reports the first wrong field in the firmware order', async () => {
        const { response } = await send({ messages: 5, brightness: 300, on: 'yes' });
        expect(response.errorMessage).toBe('on');
    });

    // A null field is ignored, like a missing one (the firmware's ArduinoJson reads null as missing)
    it('ignores null fields', async () => {
        const { statusCode, response } = await send({ brightness: null, scene: 2 });
        expect(statusCode).toBe(200);
        expect(response.data.brightness).toBe(60);
        expect(response.data.scene).toBe(2);
    });
});

// GET /api/frame: the pixels on the matrix
describe('mock frame', () => {
    // The app decodes exactly 72x12 pixels of 3 bytes
    it('answers 2592 bytes of binary data', async () => {
        const { statusCode, response, event } = await callHandler(getFrame);
        expect(statusCode).toBe(200);
        expect(event.node.res.getHeader('content-type')).toBe('application/octet-stream');
        expect(response.length).toBe(72 * 12 * 3);
    });

    // The preview is a live view, so it has to move
    it('animates the frame over time', async () => {
        vi.useFakeTimers();
        vi.setSystemTime(new Date('2026-12-01T10:00:00Z'));
        const first = Buffer.from((await callHandler(getFrame)).response);
        vi.setSystemTime(new Date('2026-12-01T10:00:01Z'));
        const second = Buffer.from((await callHandler(getFrame)).response);
        vi.useRealTimers();
        expect(first.equals(second)).toBe(false);
    });

    // The frame is what is on the matrix, before the brightness: a display that is off shows nothing
    it('answers a black frame while the display is off', async () => {
        await callHandler(postSettings, buildEvent({ on: false }));
        const { response } = await callHandler(getFrame);
        expect(response.every(level => level === 0)).toBe(true);
    });

    // The mock has something to show when the display is on
    it('lights some pixels while the display is on', async () => {
        const { response } = await callHandler(getFrame);
        expect(response.some(level => level > 0)).toBe(true);
    });
});

// The Wi-Fi routes (/api/networks, /api/connect, /api/check-connection) and the device routes come from the gizmo-common layer, which tests them
describe('mock Wi-Fi state shared with the layer', () => {
    // The strip adds its settings to the state of the layer once, and keeps them
    it('keeps the settings of the strip in the state of the layer', async () => {
        await callHandler(postSettings, buildEvent({ brightness: 99 }));
        expect(getEspMockState().settings.brightness).toBe(99);
        expect(getEspMockState()).toBe(getEspMockState());
    });
});

// The mock device introduces itself as the app (the plugin reads the name from the app.config.ts)
describe('mock device name', () => {
    // The device card of the layer shows the name the plugin gave
    it('names the mock device like the app', () => {
        nameMockDevice();
        expect(buildDevicePayload({ state: getEspMockState() }).name).toBe('Holiday Strip');
    });
});