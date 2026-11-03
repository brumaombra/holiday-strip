#include "scenes.h"

#include <time.h>

#include "font.h"

namespace {

const Color RED = {255, 0, 0};
const Color GREEN = {0, 200, 30};
const Color GOLD = {255, 150, 0};
const Color WHITE = {255, 255, 255};
const Color WARM_WHITE = {255, 190, 110};
const Color BLUE = {20, 60, 255};
const Color PINK = {255, 40, 120};
const Color SNOW = {200, 205, 225};

const Color LIGHT_COLORS[] = {RED, GREEN, BLUE, GOLD, PINK, WARM_WHITE};
const int LIGHT_COLOR_COUNT = sizeof(LIGHT_COLORS) / sizeof(LIGHT_COLORS[0]);

String message = "BUON NATALE * MERRY CHRISTMAS";
bool countdownEnabled = true;

float frand(float lo, float hi) { return lo + (hi - lo) * (esp_random() / 4294967295.0f); }

float twinkle(float t, float speed, float phase) { return 0.5f + 0.5f * sinf(t * speed + phase); }

// Sprites: '.' is transparent, other characters map to a palette color
Color spriteColor(char c, float t) {
  switch (c) {
    case 'r': return {210, 0, 0};
    case 'w': return WHITE;
    case 's': return {255, 160, 110};
    case 'b': return {130, 60, 15};
    case 'a': return {200, 150, 70};
    case 'y': return GOLD;
    case 'g': return GREEN;
    case 'n': return scaleColor(RED, 0.3f + 0.7f * twinkle(t, 6.0f, 0));  // Rudolph's nose
    default: return {0, 0, 0};
  }
}

void drawSprite(Frame &f, const char *const *rows, int rowCount, int x, int y, float t) {
  for (int r = 0; r < rowCount; r++) {
    for (int c = 0; rows[r][c]; c++) {
      if (rows[r][c] != '.') setPixel(f, x + c, y + r, spriteColor(rows[r][c], t));
    }
  }
}

struct Snowfall {
  struct Flake {
    float x, y, speed, phase;
  };
  static const int MAX_FLAKES = 48;
  Flake flakes[MAX_FLAKES];
  int count = 0;

  void begin(int n) {
    count = min(n, MAX_FLAKES);
    for (int i = 0; i < count; i++) respawn(flakes[i], true);
  }

  void respawn(Flake &fl, bool anywhere) {
    fl.x = frand(0, WIDTH);
    fl.y = anywhere ? frand(-HEIGHT, HEIGHT) : frand(-4, -0.5f);
    fl.speed = frand(1.5f, 4.5f);
    fl.phase = frand(0, 6.28f);
  }

  int columnOf(const Flake &fl, float t) const {
    return constrain((int)lroundf(fl.x + sinf(t * 1.3f + fl.phase) * 0.8f), 0, WIDTH - 1);
  }

  // ground: optional per-column snow height that grows as flakes land
  void update(float t, float dt, float *ground) {
    for (int i = 0; i < count; i++) {
      Flake &fl = flakes[i];
      fl.y += fl.speed * dt;
      int x = columnOf(fl, t);
      float limit = HEIGHT - (ground ? ground[x] : 0);
      if (fl.y >= limit) {
        if (ground) ground[x] = min(ground[x] + 0.35f, 4.0f);
        respawn(fl, false);
      }
    }
  }

  void draw(Frame &f, float t) const {
    for (int i = 0; i < count; i++) {
      const Flake &fl = flakes[i];
      if (fl.y < 0) continue;
      uint8_t v = 120 + (fl.speed - 1.5f) / 3.0f * 135;
      addPixel(f, columnOf(fl, t), (int)fl.y, {v, v, v});
    }
  }
};

struct Stars {
  struct Star {
    uint8_t x, y;
    float speed, phase;
  };
  static const int COUNT = 30;
  Star stars[COUNT];

  void begin() {
    for (auto &s : stars) {
      s.x = esp_random() % WIDTH;
      s.y = esp_random() % HEIGHT;
      s.speed = frand(1.0f, 4.0f);
      s.phase = frand(0, 6.28f);
    }
  }

  void draw(Frame &f, float t) const {
    for (const auto &s : stars) {
      float v = twinkle(t, s.speed, s.phase);
      addPixel(f, s.x, s.y, scaleColor({180, 180, 255}, v * v * 0.8f));
    }
  }
};

// Snow falls and piles up next to a small house with lit windows
class SnowScene : public Scene {
 public:
  const char *name() const override { return "Snowfall"; }

  void begin() override {
    snow.begin(40);
    for (auto &g : ground) g = 0;
  }

  void draw(Frame &f, float t, float dt) override {
    for (int y = 0; y < HEIGHT; y++) {
      Color c = blendColor({0, 0, 12}, {0, 6, 35}, y / float(HEIGHT - 1));
      for (int x = 0; x < WIDTH; x++) f[y][x] = c;
    }

    drawHouse(f, t);
    snow.update(t, dt, ground);
    settleGround(dt);
    drawGround(f);
    snow.draw(f, t);
  }

 private:
  Snowfall snow;
  float ground[WIDTH];

  void settleGround(float dt) {
    for (int x = 0; x < WIDTH - 1; x++) {
      float diff = ground[x] - ground[x + 1];
      if (fabsf(diff) > 1.0f) {
        float move = diff * 0.5f * dt;
        ground[x] -= move;
        ground[x + 1] += move;
      }
    }
  }

  void drawGround(Frame &f) {
    for (int x = 0; x < WIDTH; x++) {
      int full = (int)ground[x];
      float frac = ground[x] - full;
      for (int i = 0; i < full; i++) setPixel(f, x, HEIGHT - 1 - i, SNOW);
      if (frac > 0.05f) {
        int y = HEIGHT - 1 - full;
        if (y >= 0) f[y][x] = blendColor(f[y][x], SNOW, frac);
      }
    }
  }

  static void drawHouse(Frame &f, float t) {
    const int cx = 57;
    const Color wall = {110, 45, 15};
    const Color roof = {150, 15, 15};
    // Chimney
    for (int y = 2; y <= 4; y++) setPixel(f, 61, y, {120, 40, 30});
    setPixel(f, 61, 1, SNOW);
    // Roof with snow on its top edge
    for (int y = 3; y <= 6; y++) {
      int half = 1 + (y - 3) * 2;
      int halfAbove = y == 3 ? -1 : 1 + (y - 4) * 2;
      for (int x = cx - half; x <= cx + half; x++) {
        bool top = abs(x - cx) > halfAbove;
        setPixel(f, x, y, top ? SNOW : roof);
      }
    }
    for (int y = 7; y < HEIGHT; y++)
      for (int x = cx - 6; x <= cx + 6; x++) setPixel(f, x, y, wall);
    // Door and warm flickering windows
    for (int y = 9; y < HEIGHT; y++) setPixel(f, cx, y, {50, 20, 5});
    float flicker = 0.8f + 0.2f * sinf(t * 7.0f) * sinf(t * 2.3f);
    Color light = scaleColor({255, 170, 50}, flicker);
    for (int y = 8; y <= 9; y++) {
      setPixel(f, cx - 4, y, light);
      setPixel(f, cx - 3, y, light);
      setPixel(f, cx + 3, y, light);
      setPixel(f, cx + 4, y, light);
    }
  }
};

// A row of Christmas trees with twinkling ornaments and a star on top
class TreesScene : public Scene {
 public:
  const char *name() const override { return "Christmas trees"; }

  void begin() override {
    snow.begin(16);
    ornamentCount = 0;
    for (const auto &tree : TREES) {
      for (int y = tree.top + 1; y <= 10; y++) {
        int half = halfWidth(tree, y);
        for (int x = tree.cx - half; x <= tree.cx + half; x++) {
          if (ornamentCount < MAX_ORNAMENTS && (esp_random() % 100) < 30) {
            ornaments[ornamentCount++] = {(int8_t)x, (int8_t)y, LIGHT_COLORS[esp_random() % LIGHT_COLOR_COUNT],
                                          frand(1.5f, 5.0f), frand(0, 6.28f)};
          }
        }
      }
    }
  }

  void draw(Frame &f, float t, float dt) override {
    fillFrame(f, {0, 0, 20});
    for (int x = 0; x < WIDTH; x++) f[HEIGHT - 1][x] = {120, 125, 140};

    for (const auto &tree : TREES) {
      for (int y = tree.top; y <= 10; y++) {
        int half = halfWidth(tree, y);
        for (int x = tree.cx - half; x <= tree.cx + half; x++) {
          uint8_t shade = 90 + ((x * 7 + y * 5) % 3) * 30;
          setPixel(f, x, y, {0, shade, 20});
        }
      }
      setPixel(f, tree.cx, HEIGHT - 1, {100, 45, 10});
      float s = 0.6f + 0.4f * twinkle(t, 3.0f, tree.cx);
      setPixel(f, tree.cx, tree.top - 1, scaleColor({255, 220, 60}, s));
      if (tree.top >= 2) addPixel(f, tree.cx, tree.top - 2, scaleColor({255, 220, 60}, s * 0.25f));
    }

    for (int i = 0; i < ornamentCount; i++) {
      const auto &o = ornaments[i];
      setPixel(f, o.x, o.y, scaleColor(o.color, 0.3f + 0.7f * twinkle(t, o.speed, o.phase)));
    }

    snow.update(t, dt, nullptr);
    snow.draw(f, t);
  }

 private:
  struct Tree {
    int cx, top;
  };
  struct Ornament {
    int8_t x, y;
    Color color;
    float speed, phase;
  };
  static constexpr Tree TREES[] = {{6, 5}, {18, 1}, {31, 4}, {44, 1}, {57, 5}, {67, 2}};
  static const int MAX_ORNAMENTS = 120;
  Ornament ornaments[MAX_ORNAMENTS];
  int ornamentCount = 0;
  Snowfall snow;

  static int halfWidth(const Tree &tree, int y) { return (y - tree.top + 1) / 2; }
};
constexpr TreesScene::Tree TreesScene::TREES[];

// Scrolling greeting between two chasing garlands, with a countdown to Christmas
class GreetingScene : public Scene {
 public:
  const char *name() const override { return "Greeting"; }

  void begin() override {
    text = buildText();
    scroll = 0;
  }

  void draw(Frame &f, float t, float dt) override {
    fillFrame(f, {0, 0, 0});
    stars.draw(f, t);
    for (int y : {1, 9, 10})
      for (int x = 0; x < WIDTH; x++) f[y][x] = {0, 0, 0};

    const Color palette[] = {RED, GREEN, GOLD};
    // Each pass picks up the latest message and countdown
    scroll += SPEED * dt;
    if (scroll >= textWidth(text) + WIDTH) {
      scroll = 0;
      text = buildText();
    }
    float pos = WIDTH - scroll;
    for (size_t i = 0; i < text.length(); i++) {
      int x = (int)pos + i * GLYPH_ADVANCE;
      if (x < -GLYPH_ADVANCE || x >= WIDTH) continue;
      Color base = palette[i % 3];
      drawChar(f, text[i], x, 2, [&](int px, int py) {
        return scaleColor(base, 0.75f + 0.25f * sinf(px * 0.4f - t * 6.0f));
      });
    }

    int step = (int)(t * 5);
    for (int x = 1; x < WIDTH; x += 3) {
      int bulb = x / 3;
      Color c = LIGHT_COLORS[bulb % LIGHT_COLOR_COUNT];
      setPixel(f, x, 0, scaleColor(c, (bulb + step) % 3 == 0 ? 1.0f : 0.25f));
      setPixel(f, WIDTH - 1 - x, HEIGHT - 1, scaleColor(c, (bulb + step) % 3 == 0 ? 1.0f : 0.25f));
    }
  }

  GreetingScene() { stars.begin(); }

 private:
  static constexpr float SPEED = 16.0f;
  String text;
  float scroll = 0;
  Stars stars;

  static String buildText() {
    String result = message;
    if (!countdownEnabled) return result;
    struct tm now;
    if (!getLocalTime(&now, 0) || now.tm_year < 120) return result;

    struct tm xmas = now;
    xmas.tm_mon = 11;
    xmas.tm_mday = 25;
    xmas.tm_hour = 0;
    xmas.tm_min = 0;
    xmas.tm_sec = 0;
    if (now.tm_mon == 11 && now.tm_mday == 25) return result + " * OGGI E' NATALE!";
    struct tm today = now;
    today.tm_hour = 0;
    today.tm_min = 0;
    today.tm_sec = 0;
    long days = lround(difftime(mktime(&xmas), mktime(&today)) / 86400.0);
    if (days <= 0 || days > 60) return result;
    if (days == 1) return result + " * MANCA 1 GIORNO A NATALE!";
    return result + " * MANCANO " + String(days) + " GIORNI A NATALE!";
  }
};

// Red and white candy cane stripes sliding diagonally, with sparkles
class CandyCaneScene : public Scene {
 public:
  const char *name() const override { return "Candy cane"; }

  void draw(Frame &f, float t, float dt) override {
    for (int y = 0; y < HEIGHT; y++) {
      float shade = 0.55f + 0.45f * sinf((y + 0.5f) / HEIGHT * PI);
      for (int x = 0; x < WIDTH; x++) {
        float s = sinf((x + y - t * 8.0f) * 2 * PI / 10.0f);
        float w = constrain(s * 2.5f * 0.5f + 0.5f, 0.0f, 1.0f);
        f[y][x] = scaleColor(blendColor({255, 235, 235}, {230, 0, 0}, w), shade);
      }
    }
    for (int i = 0; i < 3; i++) {
      if (esp_random() % 100 < 15) addPixel(f, esp_random() % WIDTH, esp_random() % HEIGHT, WHITE);
    }
  }
};

// Santa's sleigh and reindeer flying across a starry sky, leaving a trail of sparkles
class SleighScene : public Scene {
 public:
  const char *name() const override { return "Santa's sleigh"; }

  SleighScene() { stars.begin(); }

  void begin() override {
    for (auto &p : trail) p.life = 0;
  }

  void draw(Frame &f, float t, float dt) override {
    fillFrame(f, {0, 0, 12});
    stars.draw(f, t);
    drawMoon(f);

    const int total = 38;
    float x0 = -total + fmodf(t * 14.0f, WIDTH + total + 12);
    int sx = (int)x0;
    int sleighY = 1 + (int)lroundf(sinf(t * 3.0f) * 0.7f);
    int deer1Y = 2 + (int)lroundf(sinf(t * 3.0f + 1.0f) * 0.7f);
    int deer2Y = 2 + (int)lroundf(sinf(t * 3.0f + 2.0f) * 0.7f);
    bool legs = ((int)(t * 6)) % 2;

    updateTrail(dt, sx, sleighY);
    drawTrail(f);

    // Harness
    for (int x = sx + 12; x < sx + 16; x++) setPixel(f, x, sleighY + 6, GOLD);
    for (int x = sx + 25; x < sx + 28; x++) setPixel(f, x, deer1Y + 4, GOLD);

    drawSprite(f, SLEIGH, 10, sx, sleighY, t);
    drawDeer(f, sx + 15, deer1Y, legs, false, t);
    drawDeer(f, sx + 28, deer2Y, !legs, true, t);
  }

 private:
  struct Particle {
    float x, y, life;
  };
  static const int MAX_PARTICLES = 40;
  Particle trail[MAX_PARTICLES];
  float spawnAccumulator = 0;
  Stars stars;

  static constexpr const char *SLEIGH[10] = {
      "....w........",
      "...rr........",
      "..rrrr.......",
      "..wwww.......",
      "..sss........",
      "r.wwww......y",
      "rrrrrrrrrrr.y",
      "rryyyyyyyyrr.",
      ".rrrrrrrrrr..",
      "yyyyyyyyyyyyy",
  };
  static constexpr const char *DEER_BODY[6] = {
      "......a.a.",
      ".......a..",
      "......bbbn",
      "w.....bb..",
      "bbbbbbbb..",
      "bbbbbbb...",
  };
  static constexpr const char *LEGS_A[2] = {".b....b.b.", "b......b.b"};
  static constexpr const char *LEGS_B[2] = {"..b...b...", "..b...b..."};

  void drawDeer(Frame &f, int x, int y, bool legPose, bool rudolph, float t) {
    for (int r = 0; r < 6; r++) {
      for (int c = 0; DEER_BODY[r][c]; c++) {
        char ch = DEER_BODY[r][c];
        if (ch == 'n' && !rudolph) ch = 'b';
        if (ch != '.') setPixel(f, x + c, y + r, spriteColor(ch, t));
      }
    }
    drawSprite(f, legPose ? LEGS_A : LEGS_B, 2, x, y + 6, t);
  }

  static void drawMoon(Frame &f) {
    for (int y = 0; y < 7; y++) {
      for (int x = 60; x < 72; x++) {
        float d1 = hypotf(x - 66.0f, y - 3.0f);
        float d2 = hypotf(x - 67.6f, y - 2.2f);
        if (d1 <= 2.8f && d2 > 2.4f) setPixel(f, x, y, {255, 235, 170});
      }
    }
  }

  void updateTrail(float dt, int sx, int sleighY) {
    for (auto &p : trail) {
      if (p.life <= 0) continue;
      p.life -= dt;
      p.y += dt * 1.5f;
    }
    spawnAccumulator += dt * 18.0f;
    while (spawnAccumulator >= 1.0f) {
      spawnAccumulator -= 1.0f;
      for (auto &p : trail) {
        if (p.life <= 0) {
          p = {sx + frand(-2.0f, 1.0f), sleighY + frand(5.0f, 9.0f), frand(0.6f, 1.4f)};
          break;
        }
      }
    }
  }

  void drawTrail(Frame &f) {
    for (const auto &p : trail) {
      if (p.life <= 0) continue;
      Color c = blendColor(GOLD, WHITE, p.life);
      addPixel(f, (int)p.x, (int)p.y, scaleColor(c, min(1.0f, p.life)));
    }
  }
};
constexpr const char *SleighScene::SLEIGH[];
constexpr const char *SleighScene::DEER_BODY[];
constexpr const char *SleighScene::LEGS_A[];
constexpr const char *SleighScene::LEGS_B[];

// Two strings of fairy lights hanging in swags, cycling through light effects
class GarlandScene : public Scene {
 public:
  const char *name() const override { return "Fairy lights"; }

  void begin() override {
    for (auto &p : phases) p = frand(0, 6.28f);
  }

  void draw(Frame &f, float t, float dt) override {
    fillFrame(f, {0, 0, 0});
    int mode = ((int)(t / 6.0f)) % 3;
    for (int g = 0; g < 2; g++) {
      int base = g == 0 ? 0 : 6;
      float offset = g == 0 ? 0 : 12;
      int bulb = 0;
      for (int x = 0; x < WIDTH; x++) {
        float u = fmodf(x + offset, SWAG) - SWAG / 2;
        int wy = base + (int)lroundf(3.0f * (1.0f - (u * u) / (SWAG * SWAG / 4)));
        setPixel(f, x, wy, {0, 35, 5});
        if (((int)(x + offset)) % 4 != 2) continue;

        int id = g * 20 + bulb++;
        Color c = LIGHT_COLORS[id % LIGHT_COLOR_COUNT];
        float v;
        if (mode == 0) {
          v = 0.15f + 0.85f * powf(twinkle(t, 2.0f, phases[id % 40]), 2);
        } else if (mode == 1) {
          v = ((id + (int)(t * 6)) % 4 == 0) ? 1.0f : 0.15f;
        } else {
          c = LIGHT_COLORS[(id + (int)(t * 2)) % LIGHT_COLOR_COUNT];
          v = 1.0f;
        }
        Color lit = scaleColor(c, v);
        setPixel(f, x, wy + 1, lit);
        Color glow = scaleColor(lit, 0.18f);
        addPixel(f, x - 1, wy + 1, glow);
        addPixel(f, x + 1, wy + 1, glow);
        addPixel(f, x, wy + 2, glow);
      }
    }
  }

 private:
  static constexpr float SWAG = 24.0f;
  float phases[40];
};

// A brick fireplace with a crackling fire and stockings hanging from the mantel
class FireplaceScene : public Scene {
 public:
  const char *name() const override { return "Fireplace"; }

  void begin() override { memset(heat, 0, sizeof(heat)); }

  void draw(Frame &f, float t, float dt) override {
    drawBricks(f);
    for (int y = FIRE_TOP - 1; y < HEIGHT; y++)
      for (int x = FIRE_X - 1; x < FIRE_X + FIRE_W + 1; x++) setPixel(f, x, y, {8, 3, 2});

    updateFire();
    for (int x = 0; x < FIRE_W; x++) {
      for (int h = 0; h < FIRE_H; h++) {
        Color c = heatColor(heat[x][h]);
        if (c.r || c.g || c.b) setPixel(f, FIRE_X + x, HEIGHT - 2 - h, c);
      }
    }
    drawLogs(f, t);

    // Mantel
    for (int x = 0; x < WIDTH; x++) {
      setPixel(f, x, 0, {90, 45, 20});
      setPixel(f, x, 1, {60, 28, 10});
    }
    const int stockingsX[] = {3, 10, 57, 64};
    for (int i = 0; i < 4; i++) drawStocking(f, stockingsX[i], i % 2 ? GREEN : Color{210, 0, 0}, t, i);
  }

 private:
  static const int FIRE_X = 19, FIRE_W = 34, FIRE_TOP = 3;
  static const int FIRE_H = HEIGHT - 1 - FIRE_TOP;
  uint8_t heat[FIRE_W][FIRE_H];

  static Color heatColor(uint8_t h) {
    uint8_t t192 = h * 191 / 255;
    uint8_t ramp = (t192 & 63) << 2;
    if (t192 >= 128) return {255, 255, ramp};
    if (t192 >= 64) return {255, ramp, 0};
    return {ramp, 0, 0};
  }

  void updateFire() {
    for (int x = 0; x < FIRE_W; x++) {
      // Flames are taller in the middle of the fireplace
      float center = 1.0f - fabsf(x - FIRE_W / 2.0f) / (FIRE_W / 2.0f);
      int cooling = 110 - (int)(center * 45);
      for (int h = 0; h < FIRE_H; h++) {
        // Cool faster higher up so the flames get pointed tips
        int cool = esp_random() % (cooling * (5 + h * 2) / 10 + 1);
        heat[x][h] = heat[x][h] > cool ? heat[x][h] - cool : 0;
      }
      for (int h = FIRE_H - 1; h >= 2; h--) {
        int left = heat[max(0, x - 1)][h - 1];
        int right = heat[min(FIRE_W - 1, x + 1)][h - 1];
        heat[x][h] = (heat[x][h - 1] * 2 + heat[x][h - 2] + left + right) / 5;
      }
      if (esp_random() % 100 < 25 + center * 35) {
        int h = esp_random() % 2;
        heat[x][h] = min(255, heat[x][h] + 160 + (int)(esp_random() % 96));
      }
    }
  }

  static void drawBricks(Frame &f) {
    for (int y = 2; y < HEIGHT; y++) {
      for (int x = 0; x < WIDTH; x++) {
        bool mortar = y % 2 == 1 || (x + (y / 2 % 2) * 3) % 6 == 0;
        Color brick = ((x / 6 + y) % 3 == 0) ? Color{60, 16, 8} : Color{48, 13, 6};
        f[y][x] = mortar ? Color{14, 12, 11} : brick;
      }
    }
  }

  static void drawLogs(Frame &f, float t) {
    for (int x = FIRE_X + 2; x < FIRE_X + FIRE_W - 2; x++) {
      float ember = 0.5f + 0.5f * sinf(t * 3.0f + x * 1.7f);
      setPixel(f, x, HEIGHT - 1, blendColor({70, 30, 10}, {255, 60, 0}, ember * 0.6f));
    }
  }

  static void drawStocking(Frame &f, int x, Color body, float t, int index) {
    static const char *const SHAPE[] = {"www..", "rrr..", "rrr..", "rrr..", "rrrrr", ".rrrr"};
    for (int r = 0; r < 6; r++) {
      for (int c = 0; c < 5; c++) {
        char ch = SHAPE[r][c];
        if (ch == '.') continue;
        Color color = ch == 'w' ? WHITE : (r == 2 ? WHITE : body);
        setPixel(f, x + c, 2 + r, color);
      }
    }
    // A little sparkle on each stocking
    if (twinkle(t, 2.0f, index * 1.9f) > 0.92f) setPixel(f, x + 1, 3, GOLD);
  }
};

// Wrapped presents fall from the sky and stack up into a pile
class GiftsScene : public Scene {
 public:
  const char *name() const override { return "Gifts"; }

  void begin() override {
    clearPile();
    falling.active = false;
    sinking = 0;
  }

  void draw(Frame &f, float t, float dt) override {
    fillFrame(f, {0, 0, 12});
    stars.draw(f, t);

    if (sinking > 0) {
      sinking -= dt;
      if (sinking <= 0) clearPile();
    }
    int offset = sinking > 0 ? (int)((1.0f - sinking / SINK_SECONDS) * HEIGHT) : 0;
    for (int y = 0; y < HEIGHT; y++)
      for (int x = 0; x < WIDTH; x++)
        if (pileUsed[y][x] && y + offset < HEIGHT) f[y + offset][x] = pile[y][x];

    if (sinking > 0) return;
    if (!falling.active) spawn();
    falling.speed += 25.0f * dt;
    falling.y += falling.speed * dt;
    int landY = landingTop(falling.x, falling.w);
    if (falling.y + falling.h >= landY) {
      falling.y = landY - falling.h;
      paintGift(falling, (int)falling.y, true, f);
      falling.active = false;
      if (landY - falling.h < 4) sinking = SINK_SECONDS;
    } else {
      paintGift(falling, (int)falling.y, false, f);
    }
  }

  GiftsScene() { stars.begin(); }

 private:
  struct Gift {
    int x, w, h;
    float y, speed;
    Color box, ribbon;
    bool active;
  };
  static constexpr float SINK_SECONDS = 1.2f;
  Color pile[HEIGHT][WIDTH];
  bool pileUsed[HEIGHT][WIDTH];
  int top[WIDTH];
  Gift falling;
  float sinking = 0;
  Stars stars;

  void clearPile() {
    memset(pileUsed, 0, sizeof(pileUsed));
    for (auto &t : top) t = HEIGHT;
  }

  int landingTop(int x, int w) const {
    int result = HEIGHT;
    for (int i = x; i < x + w; i++) result = min(result, top[i]);
    return result;
  }

  void spawn() {
    static const Color boxes[] = {{210, 0, 0}, {0, 150, 30}, {20, 60, 230}, {150, 0, 160}, {230, 120, 0}};
    static const Color ribbons[] = {GOLD, WHITE, {255, 40, 40}};
    falling.w = 5 + esp_random() % 3;
    falling.h = 3 + esp_random() % 2;
    // Prefer the lowest spots so the pile grows evenly
    int bestX = 0, bestTop = -1;
    for (int tries = 0; tries < 6; tries++) {
      int x = esp_random() % (WIDTH - falling.w + 1);
      int t = landingTop(x, falling.w);
      if (t > bestTop) bestTop = t, bestX = x;
    }
    falling.x = bestX;
    falling.y = -falling.h - 2;
    falling.speed = 2.0f;
    falling.box = boxes[esp_random() % 5];
    falling.ribbon = ribbons[esp_random() % 3];
    falling.active = true;
  }

  // Box with a ribbon cross and a bow on top; landed gifts are added to the pile
  void paintGift(const Gift &g, int y, bool land, Frame &f) {
    int cx = g.x + g.w / 2;
    // Bows don't count as support, so gifts sit flat on the boxes below
    auto put = [&](int x, int py, Color c, bool solid = true) {
      if (land) {
        if (x >= 0 && x < WIDTH && py >= 0 && py < HEIGHT) {
          pile[py][x] = c;
          pileUsed[py][x] = true;
          if (solid) top[x] = min(top[x], py);
        }
      } else {
        setPixel(f, x, py, c);
      }
    };
    for (int r = 0; r < g.h; r++)
      for (int c = 0; c < g.w; c++) {
        int x = g.x + c;
        bool ribbon = x == cx || r == g.h / 2;
        put(x, y + r, ribbon ? g.ribbon : g.box);
      }
    put(cx - 1, y - 1, g.ribbon, false);
    put(cx + 1, y - 1, g.ribbon, false);
  }
};

// Fireworks bursting over the year number, for New Year's Eve
class FireworksScene : public Scene {
 public:
  const char *name() const override { return "Fireworks"; }

  void begin() override {
    fillFrame(canvas, {0, 0, 0});
    for (auto &r : rockets) r.active = false;
    for (auto &p : particles) p.life = 0;
    nextLaunch = 0;
    year = upcomingYear();
  }

  void draw(Frame &f, float t, float dt) override {
    for (auto &row : canvas)
      for (auto &p : row) p = scaleColor(p, 0.72f);

    nextLaunch -= dt;
    if (nextLaunch <= 0) {
      launch();
      nextLaunch = frand(0.4f, 1.1f);
    }
    updateRockets(dt);
    updateParticles(dt);

    fillFrame(f, {0, 0, 6});
    if (year > 0) {
      String text = String(year);
      int x0 = (WIDTH - textWidth(text) + 1) / 2;
      for (size_t i = 0; i < text.length(); i++)
        drawChar(f, text[i], x0 + i * GLYPH_ADVANCE, 4,
                 [&](int px, int py) { return scaleColor(GOLD, 0.35f + 0.1f * sinf(t * 2 + px * 0.3f)); });
    }
    for (int y = 0; y < HEIGHT; y++)
      for (int x = 0; x < WIDTH; x++) addPixel(f, x, y, canvas[y][x]);
  }

 private:
  struct Rocket {
    float x, y, vy, burstY;
    bool active;
  };
  struct Particle {
    float x, y, vx, vy, life, maxLife;
    Color color;
  };
  static const int MAX_ROCKETS = 4, MAX_PARTICLES = 160;
  Rocket rockets[MAX_ROCKETS];
  Particle particles[MAX_PARTICLES];
  Frame canvas;
  float nextLaunch = 0;
  int year = 0;

  static int upcomingYear() {
    struct tm now;
    if (!getLocalTime(&now, 0) || now.tm_year < 120) return 0;
    return now.tm_year + 1900 + (now.tm_mon >= 10 ? 1 : 0);
  }

  void launch() {
    for (auto &r : rockets) {
      if (r.active) continue;
      r = {frand(4, WIDTH - 4), (float)HEIGHT, frand(-16.0f, -12.0f), frand(1.5f, 4.5f), true};
      return;
    }
  }

  void updateRockets(float dt) {
    for (auto &r : rockets) {
      if (!r.active) continue;
      r.y += r.vy * dt;
      addPixel(canvas, (int)r.x, (int)r.y, {255, 180, 80});
      if (r.y <= r.burstY) {
        r.active = false;
        burst(r.x, r.y);
      }
    }
  }

  void burst(float x, float y) {
    Color color = hsvColor(esp_random() % 256, 230, 255);
    Color second = hsvColor(esp_random() % 256, 200, 255);
    int count = 0;
    for (auto &p : particles) {
      if (p.life > 0) continue;
      float angle = frand(0, 2 * PI);
      float speed = frand(4.0f, 11.0f);
      float life = frand(0.8f, 1.6f);
      // Squash vertically: the matrix is much wider than it is tall
      p = {x, y, cosf(angle) * speed, sinf(angle) * speed * 0.6f, life, life, count % 3 ? color : second};
      if (++count >= 36) break;
    }
    for (int dy = -1; dy <= 1; dy++)
      for (int dx = -1; dx <= 1; dx++) addPixel(canvas, (int)x + dx, (int)y + dy, WHITE);
  }

  void updateParticles(float dt) {
    for (auto &p : particles) {
      if (p.life <= 0) continue;
      p.life -= dt;
      p.vx *= 1.0f - 1.8f * dt;
      p.vy = p.vy * (1.0f - 1.8f * dt) + 5.0f * dt;
      p.x += p.vx * dt;
      p.y += p.vy * dt;
      float k = p.life / p.maxLife;
      // Sparkle as they fade out
      if (k < 0.35f && esp_random() % 3 == 0) continue;
      addPixel(canvas, (int)p.x, (int)p.y, scaleColor(blendColor(p.color, WHITE, k > 0.85f ? 0.6f : 0), k));
    }
  }
};

SnowScene snowScene;
TreesScene treesScene;
GreetingScene greetingScene;
SleighScene sleighScene;
CandyCaneScene candyCaneScene;
GarlandScene garlandScene;
FireplaceScene fireplaceScene;
GiftsScene giftsScene;
FireworksScene fireworksScene;

Scene *const ALL_SCENES[] = {&greetingScene, &snowScene,      &treesScene,     &sleighScene, &fireplaceScene,
                             &garlandScene,  &giftsScene,     &candyCaneScene, &fireworksScene};

}  // namespace

int scenes::count() { return sizeof(ALL_SCENES) / sizeof(ALL_SCENES[0]); }

Scene *scenes::get(int index) { return ALL_SCENES[index]; }

void scenes::setMessage(const String &text) { message = text; }

void scenes::setCountdown(bool enabled) { countdownEnabled = enabled; }
