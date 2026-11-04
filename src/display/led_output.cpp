#include "led_output.h"

#include "config/config.h"

namespace {
	// Gamma 2 curve, 16 bits, so the dim levels stay smooth
	uint16_t gamma16[256];

	// Apply the gamma curve and the brightness to a channel
	uint8_t toOutput(uint8_t value, uint8_t brightness) {
		return (uint32_t(gamma16[value]) * brightness + 32767) / 65535;
	}
}

// Build the gamma table
void initLedOutput() {
	for (int i = 0; i < 256; i++) {
		gamma16[i] = uint32_t(i) * i * 65535 / (255 * 255);
	}
}

// Get the position of a pixel in the LED chain (x 0..71 from the left, y 0..11 from the top): the matrix is two 72 x 6 halves, the top one rotated by 180 degrees
int getPixelIndex(int x, int y) {
	if (y >= 6) {
		return x * 6 + (y - 6);
	}
	return 432 + (WIDTH - 1 - x) * 6 + (5 - y);
}

// Fill the GRB bytes of the chain from a frame, dimmed to stay within the power limit, and return the estimated current
uint32_t buildLedOutput(const Frame &frame, uint8_t brightness, uint32_t maxMilliamps, uint8_t *output) {
	const uint32_t idleMilliamps = LED_COUNT * config::IDLE_MILLIAMPS_PER_LED;
	uint32_t channelSum = 0;

	// Every pixel, in chain order, gamma corrected and scaled by the brightness
	for (int y = 0; y < HEIGHT; y++) {
		for (int x = 0; x < WIDTH; x++) {
			const Color &color = frame[y][x];
			uint8_t *pixel = output + getPixelIndex(x, y) * 3;
			pixel[0] = toOutput(color.g, brightness);
			pixel[1] = toOutput(color.r, brightness);
			pixel[2] = toOutput(color.b, brightness);
			channelSum += pixel[0] + pixel[1] + pixel[2];
		}
	}

	// Dim the whole frame when its estimated current would go over the limit
	uint32_t activeMilliamps = channelSum * config::MILLIAMPS_PER_CHANNEL / 255;
	if (idleMilliamps + activeMilliamps > maxMilliamps && activeMilliamps > 0) {
		const uint32_t budget = maxMilliamps > idleMilliamps ? maxMilliamps - idleMilliamps : 0;
		const uint32_t factor = budget * 256 / activeMilliamps;
		for (size_t i = 0; i < LED_BYTES; i++) {
			output[i] = (output[i] * factor) >> 8;
		}
		activeMilliamps = activeMilliamps * factor / 256;
	}

	// Return the estimated current of the frame
	return idleMilliamps + activeMilliamps;
}