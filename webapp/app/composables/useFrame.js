// Size of the matrix, and of its frame (3 bytes per pixel: R, G, B)
export const FRAME_WIDTH = 72;
export const FRAME_HEIGHT = 12;
export const FRAME_BYTES = FRAME_WIDTH * FRAME_HEIGHT * 3;

// Size of one LED on the canvas, in canvas pixels
export const LED_CELL_SIZE = 12;

// Colors of the preview (the matrix is always dark, in light mode too)
const BACKGROUND_COLOR = '#0a0b10';
const OFF_LED_COLOR = '#191b25';

// Level under which a color channel sum looks like an LED that is off
const OFF_LEVEL_THRESHOLD = 12;

// Decode the bytes of GET /api/frame: null when they are not exactly one frame
export const decodeFrame = buffer => {
    if (!buffer) return null;
    const bytes = buffer instanceof Uint8Array ? buffer : new Uint8Array(buffer);
    return bytes.length === FRAME_BYTES ? bytes : null;
};

// Read the color of a pixel of the frame, row by row from the top-left corner
export const readPixel = ({ frame, x, y }) => {
    const offset = ((y * FRAME_WIDTH) + x) * 3;
    return { red: frame[offset], green: frame[offset + 1], blue: frame[offset + 2] };
};

// Check if the color of a pixel is dark enough to show the LED as off
export const isPixelOff = ({ red, green, blue }) => {
    return red + green + blue < OFF_LEVEL_THRESHOLD;
};

// Get the size of the canvas that draws a frame
export const getCanvasSize = (cellSize = LED_CELL_SIZE) => ({
    width: FRAME_WIDTH * cellSize,
    height: FRAME_HEIGHT * cellSize
});

// Draw a frame as round LEDs on a dark background (without a frame, only the dark LEDs are drawn)
export const drawFrame = ({ context, frame = null, cellSize = LED_CELL_SIZE }) => {
    const { width, height } = getCanvasSize(cellSize);
    const radius = cellSize * 0.38;

    // Paint the background
    context.globalAlpha = 1;
    context.fillStyle = BACKGROUND_COLOR;
    context.fillRect(0, 0, width, height);

    // Draw every LED, row by row
    for (let y = 0; y < FRAME_HEIGHT; y += 1) {
        for (let x = 0; x < FRAME_WIDTH; x += 1) {
            const centerX = (x + 0.5) * cellSize;
            const centerY = (y + 0.5) * cellSize;
            const pixel = frame ? readPixel({ frame, x, y }) : null;

            // Draw a dark LED when it is off
            if (!pixel || isPixelOff(pixel)) {
                context.fillStyle = OFF_LED_COLOR;
                context.beginPath();
                context.arc(centerX, centerY, radius, 0, Math.PI * 2);
                context.fill();
                continue;
            }

            // Draw a soft halo around a lit LED
            const color = `rgb(${pixel.red}, ${pixel.green}, ${pixel.blue})`;
            context.fillStyle = color;
            context.globalAlpha = 0.28;
            context.beginPath();
            context.arc(centerX, centerY, radius * 1.45, 0, Math.PI * 2);
            context.fill();

            // Draw the lit LED
            context.globalAlpha = 1;
            context.beginPath();
            context.arc(centerX, centerY, radius, 0, Math.PI * 2);
            context.fill();
        }
    }
};
