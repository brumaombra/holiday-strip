import { describe, expect, it } from 'vitest';
import { FRAME_BYTES, FRAME_HEIGHT, FRAME_WIDTH, decodeFrame, drawFrame, getCanvasSize, isPixelOff, readPixel } from '~/composables/useFrame.js';

// Build a frame with the given pixels lit ({ x, y, red, green, blue })
const buildFrame = (pixels = []) => {
    const frame = new Uint8Array(FRAME_BYTES);
    for (const { x, y, red, green, blue } of pixels) {
        const offset = ((y * FRAME_WIDTH) + x) * 3;
        frame.set([red, green, blue], offset);
    }
    return frame;
};

// Build a canvas context that records what is drawn
const buildContext = () => {
    const calls = { fills: [], arcs: [], rects: [] };
    const context = {
        fillStyle: '',
        globalAlpha: 1,
        fillRect: (...rectangle) => calls.rects.push(rectangle),
        beginPath: () => {},
        arc: (...circle) => calls.arcs.push(circle),
        fill: () => calls.fills.push({ color: context.fillStyle, alpha: context.globalAlpha })
    };
    return { context, calls };
};

// Frame of the device: 72x12 pixels of 3 bytes
describe('decodeFrame', () => {
    // The device sends exactly 2592 bytes
    it('accepts the bytes of one frame', () => {
        const buffer = new Uint8Array(FRAME_BYTES).buffer;
        expect(decodeFrame(buffer)).toHaveLength(2592);
    });

    // A half frame (a cut connection) used to be drawn with the rest of the matrix black or garbled
    it('refuses bytes that are not exactly one frame', () => {
        expect(decodeFrame(new ArrayBuffer(100))).toBeNull();
        expect(decodeFrame(new ArrayBuffer(FRAME_BYTES + 1))).toBeNull();
        expect(decodeFrame(new ArrayBuffer(0))).toBeNull();
    });

    // No answer is no frame
    it('refuses an empty answer', () => {
        expect(decodeFrame(null)).toBeNull();
        expect(decodeFrame(undefined)).toBeNull();
    });

    // A typed array from the fetch is used as it is
    it('accepts a typed array', () => {
        expect(decodeFrame(new Uint8Array(FRAME_BYTES))).toHaveLength(FRAME_BYTES);
    });
});

// Pixels are stored row by row from the top-left corner
describe('readPixel', () => {
    // The first pixel is the top-left one, the last is the bottom-right one
    it('reads the corners of the frame', () => {
        const frame = buildFrame([{ x: 0, y: 0, red: 1, green: 2, blue: 3 }, { x: 71, y: 11, red: 4, green: 5, blue: 6 }]);
        expect(readPixel({ frame, x: 0, y: 0 })).toEqual({ red: 1, green: 2, blue: 3 });
        expect(readPixel({ frame, x: 71, y: 11 })).toEqual({ red: 4, green: 5, blue: 6 });
    });

    // The second row starts after the 72 pixels of the first
    it('reads rows of 72 pixels', () => {
        const frame = buildFrame([{ x: 5, y: 1, red: 9, green: 8, blue: 7 }]);
        expect(frame[((1 * 72) + 5) * 3]).toBe(9);
        expect(readPixel({ frame, x: 5, y: 1 })).toEqual({ red: 9, green: 8, blue: 7 });
    });
});

// Dark pixels are drawn as LEDs that are off
describe('isPixelOff', () => {
    // Black is off, anything with some light is on
    it('tells dark pixels from lit ones', () => {
        expect(isPixelOff({ red: 0, green: 0, blue: 0 })).toBe(true);
        expect(isPixelOff({ red: 2, green: 2, blue: 2 })).toBe(true);
        expect(isPixelOff({ red: 255, green: 0, blue: 0 })).toBe(false);
    });
});

// Canvas size follows the matrix
describe('getCanvasSize', () => {
    // 72 columns and 12 rows of square cells
    it('has one cell per LED', () => {
        expect(getCanvasSize(10)).toEqual({ width: 720, height: 120 });
    });
});

// The canvas drawing of the live preview
describe('drawFrame', () => {
    // Every LED is a circle, lit or not, on a dark background
    it('draws one round LED for each pixel on a dark background', () => {
        const { context, calls } = buildContext();
        drawFrame({ context, frame: buildFrame(), cellSize: 10 });
        expect(calls.rects).toEqual([[0, 0, 720, 120]]);
        expect(calls.arcs).toHaveLength(FRAME_WIDTH * FRAME_HEIGHT);
    });

    // A lit LED has its own color, and a halo around it
    it('draws a lit pixel with its color and a halo', () => {
        const { context, calls } = buildContext();
        drawFrame({ context, frame: buildFrame([{ x: 2, y: 3, red: 255, green: 100, blue: 0 }]), cellSize: 10 });
        const litFills = calls.fills.filter(fill => fill.color === 'rgb(255, 100, 0)');
        expect(litFills).toHaveLength(2);
        expect(litFills[0].alpha).toBeLessThan(1);
        expect(litFills[1].alpha).toBe(1);
        expect(calls.arcs).toHaveLength((FRAME_WIDTH * FRAME_HEIGHT) + 1);
    });

    // The circle of a pixel is centered in its cell
    it('centers the LED in its cell', () => {
        const { context, calls } = buildContext();
        drawFrame({ context, frame: buildFrame([{ x: 2, y: 3, red: 255, green: 255, blue: 255 }]), cellSize: 10 });
        expect(calls.arcs).toContainEqual([25, 35, 3.8, 0, Math.PI * 2]);
    });

    // Before the first frame arrives, the matrix is drawn dark
    it('draws only dark LEDs without a frame', () => {
        const { context, calls } = buildContext();
        drawFrame({ context, frame: null, cellSize: 10 });
        expect(calls.arcs).toHaveLength(FRAME_WIDTH * FRAME_HEIGHT);
        expect(new Set(calls.fills.map(fill => fill.color)).size).toBe(1);
    });
});
