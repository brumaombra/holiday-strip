#include "display.h"

#include <driver/rmt_tx.h>

#include "config/config.h"
#include "display/led_output.h"

namespace {
	// RMT at 40 MHz: one tick is 25 ns
	constexpr uint32_t RMT_RESOLUTION_HZ = 40 * 1000 * 1000;
	constexpr size_t RMT_MEMORY_SYMBOLS = 512; // The ESP32 RMT has no DMA: the channel takes all 8 memory blocks (64 symbols each), refilled half at a time from the interrupt, every 320 us
	constexpr int RMT_INTERRUPT_PRIORITY = 3; // Highest level the driver allows, so the Wi-Fi interrupts don't delay a refill and break the frame
	constexpr uint32_t LATCH_MICROSECONDS = 300; // Line low before a new frame (PROTOCOL.md)

	// WS2812B bits (PROTOCOL.md): 0 is 400 ns high then 850 ns low, 1 is 800 ns high then 450 ns low
	constexpr uint16_t BIT0_HIGH_TICKS = 16;
	constexpr uint16_t BIT0_LOW_TICKS = 34;
	constexpr uint16_t BIT1_HIGH_TICKS = 32;
	constexpr uint16_t BIT1_LOW_TICKS = 18;

	rmt_channel_handle_t channel = nullptr;
	rmt_encoder_handle_t encoder = nullptr;
	uint8_t ledBytes[LED_BYTES];
	uint32_t milliamps = 0;

	// Build one RMT symbol: a high time, then a low time
	rmt_symbol_word_t makeSymbol(uint16_t highTicks, uint16_t lowTicks) {
		rmt_symbol_word_t symbol = {};
		symbol.level0 = 1;
		symbol.duration0 = highTicks;
		symbol.level1 = 0;
		symbol.duration1 = lowTicks;
		return symbol;
	}
}

// Start the RMT channel on the data pin, with an encoder that turns every byte into WS2812B bits
void setupDisplay() {
	initLedOutput();

	// TX channel on the data pin
	rmt_tx_channel_config_t channelConfig = {};
	channelConfig.gpio_num = (gpio_num_t)config::DATA_PIN;
	channelConfig.clk_src = RMT_CLK_SRC_DEFAULT;
	channelConfig.resolution_hz = RMT_RESOLUTION_HZ;
	channelConfig.mem_block_symbols = RMT_MEMORY_SYMBOLS;
	channelConfig.trans_queue_depth = 1;
	channelConfig.intr_priority = RMT_INTERRUPT_PRIORITY;
	ESP_ERROR_CHECK(rmt_new_tx_channel(&channelConfig, &channel));

	// Bytes encoder: most significant bit first, as the LEDs read it
	rmt_bytes_encoder_config_t encoderConfig = {};
	encoderConfig.bit0 = makeSymbol(BIT0_HIGH_TICKS, BIT0_LOW_TICKS);
	encoderConfig.bit1 = makeSymbol(BIT1_HIGH_TICKS, BIT1_LOW_TICKS);
	encoderConfig.flags.msb_first = 1;
	ESP_ERROR_CHECK(rmt_new_bytes_encoder(&encoderConfig, &encoder));
	ESP_ERROR_CHECK(rmt_enable(channel));
}

// Send a frame, once the previous one is out (its bytes are only rewritten then)
void showFrame(const Frame &frame, uint8_t brightness, uint32_t maxMilliamps) {
	rmt_tx_wait_all_done(channel, portMAX_DELAY);
	milliamps = buildLedOutput(frame, brightness, maxMilliamps, ledBytes);

	// Latch the previous frame, then send this one in the background
	delayMicroseconds(LATCH_MICROSECONDS);
	rmt_transmit_config_t transmitConfig = {};
	rmt_transmit(channel, encoder, ledBytes, sizeof(ledBytes), &transmitConfig);
}

// Get the estimated current of the last frame
uint32_t getEstimatedMilliamps() {
	return milliamps;
}