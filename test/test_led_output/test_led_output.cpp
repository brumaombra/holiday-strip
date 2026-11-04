#include <Arduino.h>
#include <unity.h>

#include "config/config.h"
#include "display/led_output.h"

namespace {
	// Frame under test, and the bytes sent to the chain
	Frame frame;
	uint8_t output[LED_BYTES];

	// Fill the whole frame with one color
	void fillWith(Color color) {
		fillFrame(frame, color);
	}

	// Get the bytes of the pixel at a position on screen
	const uint8_t *getPixelBytes(int x, int y) {
		return output + getPixelIndex(x, y) * 3;
	}
}

// Start each test with a black frame and the gamma table built
void setUp() {
	initLedOutput();
	fillWith({ 0, 0, 0 });
	memset(output, 0, sizeof(output));
}

// Nothing to clean up after each test
void tearDown() {}

/******************************** Chain order ********************************/

// The corners land where PROTOCOL.md measured them
void test_corners_match_the_protocol() {
	TEST_ASSERT_EQUAL_INT(0, getPixelIndex(0, 6));
	TEST_ASSERT_EQUAL_INT(431, getPixelIndex(71, 11));
	TEST_ASSERT_EQUAL_INT(432, getPixelIndex(71, 5));
	TEST_ASSERT_EQUAL_INT(863, getPixelIndex(0, 0));
}

// Every pixel has its own LED, and every LED is used
void test_every_pixel_has_its_own_led() {
	bool isUsed[LED_COUNT] = {};

	// Visit every pixel of the screen
	for (int y = 0; y < HEIGHT; y++) {
		for (int x = 0; x < WIDTH; x++) {
			const int index = getPixelIndex(x, y);
			TEST_ASSERT_TRUE(index >= 0 && index < LED_COUNT);
			TEST_ASSERT_FALSE(isUsed[index]);
			isUsed[index] = true;
		}
	}
}

// The bottom half runs down each column, the top half up (it is rotated by 180 degrees)
void test_columns_run_in_opposite_directions() {
	TEST_ASSERT_EQUAL_INT(getPixelIndex(10, 6) + 1, getPixelIndex(10, 7));
	TEST_ASSERT_EQUAL_INT(getPixelIndex(10, 5) + 1, getPixelIndex(10, 4));
}

/******************************** Colors ********************************/

// The LEDs read green, red, blue
void test_bytes_are_in_grb_order() {
	setPixel(frame, 0, 6, { 255, 0, 0 });
	buildLedOutput(frame, 255, config::MAX_MAX_MILLIAMPS, output);
	TEST_ASSERT_EQUAL_UINT8(0, getPixelBytes(0, 6)[0]);
	TEST_ASSERT_EQUAL_UINT8(255, getPixelBytes(0, 6)[1]);
	TEST_ASSERT_EQUAL_UINT8(0, getPixelBytes(0, 6)[2]);
}

// Gamma 2: half the value gives about a quarter of the light
void test_gamma_darkens_the_mid_tones() {
	setPixel(frame, 5, 2, { 128, 128, 128 });
	buildLedOutput(frame, 255, config::MAX_MAX_MILLIAMPS, output);
	TEST_ASSERT_UINT8_WITHIN(1, 64, getPixelBytes(5, 2)[0]);
}

// Brightness 0 turns every LED off
void test_brightness_zero_is_black() {
	fillWith({ 255, 255, 255 });
	buildLedOutput(frame, 0, config::MAX_MAX_MILLIAMPS, output);
	for (size_t i = 0; i < LED_BYTES; i++) {
		TEST_ASSERT_EQUAL_UINT8(0, output[i]);
	}
}

/******************************** Power limit ********************************/

// A black frame only draws the idle current of the LEDs
void test_black_frame_draws_the_idle_current() {
	TEST_ASSERT_EQUAL_UINT32(LED_COUNT * config::IDLE_MILLIAMPS_PER_LED, buildLedOutput(frame, 255, config::DEFAULT_MAX_MILLIAMPS, output));
}

// Full white would draw far more than the limit, so the frame is dimmed to fit it
void test_bright_frame_is_dimmed_to_the_limit() {
	fillWith({ 255, 255, 255 });

	// Build with the default 3 A limit
	const uint32_t milliamps = buildLedOutput(frame, 255, config::DEFAULT_MAX_MILLIAMPS, output);

	// Expect the estimate within the limit, and the LEDs dimmed but not off
	TEST_ASSERT_LESS_OR_EQUAL_UINT32(config::DEFAULT_MAX_MILLIAMPS, milliamps);
	TEST_ASSERT_TRUE(output[0] > 0 && output[0] < 255);
}

// A frame under the limit is sent as it is
void test_frame_under_the_limit_is_not_dimmed() {
	setPixel(frame, 20, 3, { 255, 255, 255 });
	const uint32_t milliamps = buildLedOutput(frame, 255, config::DEFAULT_MAX_MILLIAMPS, output);
	TEST_ASSERT_EQUAL_UINT8(255, getPixelBytes(20, 3)[0]);
	TEST_ASSERT_EQUAL_UINT32(LED_COUNT * config::IDLE_MILLIAMPS_PER_LED + 3 * config::MILLIAMPS_PER_CHANNEL, milliamps);
}

// Run all the LED output tests
int main() {
	UNITY_BEGIN();
	RUN_TEST(test_corners_match_the_protocol);
	RUN_TEST(test_every_pixel_has_its_own_led);
	RUN_TEST(test_columns_run_in_opposite_directions);
	RUN_TEST(test_bytes_are_in_grb_order);
	RUN_TEST(test_gamma_darkens_the_mid_tones);
	RUN_TEST(test_brightness_zero_is_black);
	RUN_TEST(test_black_frame_draws_the_idle_current);
	RUN_TEST(test_bright_frame_is_dimmed_to_the_limit);
	RUN_TEST(test_frame_under_the_limit_is_not_dimmed);
	return UNITY_END();
}