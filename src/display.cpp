#include "display.h"

#include <driver/rmt.h>

namespace {

const gpio_num_t DATA_PIN = GPIO_NUM_13;
const rmt_channel_t RMT_CH = RMT_CHANNEL_0;
const int NUM_LEDS = WIDTH * HEIGHT;

// WS2812B at 800 kHz, RMT ticks of 25 ns (80 MHz / 2)
const rmt_item32_t WS_BIT0 = {{{16, 1, 34, 0}}};  // 400 ns high, 850 ns low
const rmt_item32_t WS_BIT1 = {{{32, 1, 18, 0}}};  // 800 ns high, 450 ns low

// WS2812B: about 20 mA per color channel at full drive, about 1 mA quiescent per LED
const uint32_t MA_PER_CHANNEL = 20;
const uint32_t IDLE_MA = NUM_LEDS * 1;

uint8_t out[NUM_LEDS * 3];
uint16_t gamma16[256];
uint32_t milliamps = 0;

void IRAM_ATTR translate(const void *src, rmt_item32_t *dest, size_t srcSize, size_t wantedNum,
                         size_t *translatedSize, size_t *itemNum) {
  size_t size = 0, num = 0;
  const uint8_t *psrc = (const uint8_t *)src;
  while (size < srcSize && num < wantedNum) {
    for (int i = 7; i >= 0; i--) {
      dest->val = (*psrc & (1 << i)) ? WS_BIT1.val : WS_BIT0.val;
      dest++;
    }
    num += 8;
    size++;
    psrc++;
  }
  *translatedSize = size;
  *itemNum = num;
}

// The matrix is two 72x6 halves; the top one is the bottom one rotated by 180 degrees
int pixelIndex(int x, int y) {
  if (y >= 6) return x * 6 + (y - 6);
  return 432 + (WIDTH - 1 - x) * 6 + (5 - y);
}

inline uint8_t toOutput(uint8_t value, uint8_t brightness) {
  return (uint32_t(gamma16[value]) * brightness + 32767) / 65535;
}

}  // namespace

void display::begin() {
  for (int i = 0; i < 256; i++) gamma16[i] = uint32_t(i) * i * 65535 / (255 * 255);

  rmt_config_t config = RMT_DEFAULT_CONFIG_TX(DATA_PIN, RMT_CH);
  config.clk_div = 2;
  config.mem_block_num = 4;
  rmt_config(&config);
  rmt_driver_install(RMT_CH, 0, 0);
  rmt_translator_init(RMT_CH, translate);
}

void display::show(const Frame &frame, uint8_t brightness, uint32_t maxMilliamps) {
  rmt_wait_tx_done(RMT_CH, portMAX_DELAY);

  uint32_t channelSum = 0;
  for (int y = 0; y < HEIGHT; y++) {
    for (int x = 0; x < WIDTH; x++) {
      const Color &c = frame[y][x];
      uint8_t *p = out + pixelIndex(x, y) * 3;
      p[0] = toOutput(c.g, brightness);
      p[1] = toOutput(c.r, brightness);
      p[2] = toOutput(c.b, brightness);
      channelSum += p[0] + p[1] + p[2];
    }
  }

  uint32_t activeMa = channelSum * MA_PER_CHANNEL / 255;
  if (IDLE_MA + activeMa > maxMilliamps && activeMa > 0) {
    uint32_t budget = maxMilliamps > IDLE_MA ? maxMilliamps - IDLE_MA : 0;
    uint32_t factor = budget * 256 / activeMa;
    for (auto &v : out) v = (v * factor) >> 8;
    activeMa = activeMa * factor / 256;
  }
  milliamps = IDLE_MA + activeMa;

  // Latch time between frames
  delayMicroseconds(300);
  rmt_write_sample(RMT_CH, out, sizeof(out), false);
}

uint32_t display::estimatedMilliamps() { return milliamps; }
