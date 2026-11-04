import { IncomingMessage, ServerResponse } from 'node:http';
import { Socket } from 'node:net';
import { beforeEach, describe, expect, it, vi } from 'vitest';
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
const { SCENE_NAMES, getEspMockState } = await import('../../server/utils/esp-mock-state.js');
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

// Start every test from the demo device the dev server boots with
beforeEach(() => {
    resetGizmoMockState();
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
            scene: -1,
            maxMilliamps: 3000,
            countdown: true,
            message: 'BUON NATALE * MERRY CHRISTMAS',
            scenes: SCENE_NAMES,
            minMaxMilliamps: 1000,
            maxMaxMilliamps: 8000,
            maxMessageLength: 200,
            ssid: 'BrumaLab WiFi'
        });
        expect(response.data.scenes).toHaveLength(9);
        expect(response.data.currentScene).toBeGreaterThanOrEqual(0);
        expect(response.data.currentScene).toBeLessThan(9);
        expect(response.data.uptimeSeconds).toBeGreaterThanOrEqual(0);
    });

    // A chosen scene is the one on screen, while the rotation moves on by itself
    it('shows the chosen scene as the current one', async () => {
        await callHandler(postSettings, buildEvent({ scene: 4 }));
        const { response } = await callHandler(getState);
        expect(response.data.currentScene).toBe(4);
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
        const { statusCode, response } = await send({ brightness: 120, scene: 2, on: false });
        expect(statusCode).toBe(200);
        expect(response.data).toMatchObject({ brightness: 120, scene: 2, on: false, scenes: SCENE_NAMES });
    });

    // Any subset of the fields is accepted, and the others keep their value
    it('changes only the fields that are sent', async () => {
        await send({ countdown: false });
        const { response } = await send({});
        expect(response.data).toMatchObject({ countdown: false, brightness: 60, on: true, maxMilliamps: 3000 });
    });

    // The device stores the message in capitals
    it('stores the message in capitals', async () => {
        const { response } = await send({ message: 'Merry Christmas <3' });
        expect(response.data.message).toBe('MERRY CHRISTMAS <3');
    });

    // The mock used to save any field it got, while the firmware ignores the unknown ones
    it('ignores the fields it does not know', async () => {
        const { response } = await send({ ssid: 'elsewhere', uptimeSeconds: 99999, scenes: [], currentScene: 8 });
        expect(response.data.ssid).toBe('BrumaLab WiFi');
        expect(response.data.scenes).toEqual(SCENE_NAMES);
        expect(response.data.uptimeSeconds).toBeLessThan(99999);
    });

    // Every field refuses the wrong type, with its name in the error
    it.each([
        ['on', 1],
        ['on', 'true'],
        ['brightness', '60'],
        ['brightness', 12.5],
        ['scene', '1'],
        ['scene', 1.5],
        ['maxMilliamps', '3000'],
        ['maxMilliamps', 2000.5],
        ['countdown', 'yes'],
        ['countdown', 0],
        ['message', 42]
    ])('refuses %s = %j (wrong type)', async (field, value) => {
        const { statusCode, response } = await send({ [field]: value });
        expect(statusCode).toBe(400);
        expect(response).toEqual({ errorId: 'invalid_settings_field', errorMessage: field, data: null });
    });

    // Every field refuses a value out of range
    it.each([
        ['brightness', -1],
        ['brightness', 256],
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
        ['scene', -1],
        ['scene', 8],
        ['maxMilliamps', 1000],
        ['maxMilliamps', 8000]
    ])('accepts %s = %j (edge of the range)', async (field, value) => {
        const { statusCode, response } = await send({ [field]: value });
        expect(statusCode).toBe(200);
        expect(response.data[field]).toBe(value);
    });

    // The message is limited to 200 bytes
    it('refuses a message longer than 200 bytes, and accepts exactly 200', async () => {
        const exact = await send({ message: 'A'.repeat(200) });
        const tooLong = await send({ message: 'A'.repeat(201) });
        expect(exact.statusCode).toBe(200);
        expect(tooLong.response).toEqual({ errorId: 'invalid_settings_field', errorMessage: 'message', data: null });
    });

    // The device counts bytes (strlen), so accented letters weigh two
    it('counts the message in bytes', async () => {
        const { statusCode } = await send({ message: 'È'.repeat(101) });
        expect(statusCode).toBe(400);
    });

    // A refused body changes nothing, even the valid fields in it
    it('applies nothing when one field is wrong', async () => {
        await send({ brightness: 10, scene: 99 });
        const { response } = await callHandler(getState);
        expect(response.data.brightness).toBe(60);
        expect(getEspMockState().settings.scene).toBe(-1);
    });

    // Several wrong fields report the first one in the firmware order (on, countdown, brightness, scene, maxMilliamps, message)
    it('reports the first wrong field in the firmware order', async () => {
        const { response } = await send({ message: 5, brightness: 300, countdown: 'yes' });
        expect(response.errorMessage).toBe('countdown');
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
    // The state endpoint reads the saved network of the layer's mock, so a network saved by the layer's routes shows up here
    it('shows the network saved by the Wi-Fi routes of the layer', async () => {
        getEspMockState().savedSsid = 'Warehouse Guest';
        const { response } = await callHandler(getState);
        expect(response.data.ssid).toBe('Warehouse Guest');
    });

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