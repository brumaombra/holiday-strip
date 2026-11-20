#include "scene_list.h"

#include "scenes/effects.h"

namespace {

	// A bunny hopping across a meadow, leaving painted eggs hidden in the grass behind it (a 14 second loop)
	class BunnyScene : public Scene {
	public:
		// Get the id and the name of the scene
		const char *id() const override { return "bunny"; }
		const char *name() const override { return "Egg hunt"; }

		// Restart the hunt with new egg colors
		void begin() override {
			loopTime = 0;
			colorShift = esp_random() % EGG_COLOR_COUNT;
		}

		// Draw the sky, the eggs, the bunny and the blades of grass in front of them
		void draw(Frame &f, float t, float dt) override {
			// Run the loop, starting again with new colors
			loopTime += dt;
			if (loopTime >= LOOP_SECONDS) {
				begin();
			}

			// Draw the dawn sky, a pale sun and the ground
			drawSky(f, t);
			for (int x = 0; x < WIDTH; x++) {
				setPixel(f, x, 10, { 30, 130, 40 });
				setPixel(f, x, 11, { 14, 90, 30 });
			}

			// Get how far the hunt is: the eggs fade away at the end of the loop
			const float fade = constrain((LOOP_SECONDS - loopTime) / FADE_SECONDS, 0.0f, 1.0f);

			// Draw the eggs the bunny has left behind (one per landing)
			const float hops = loopTime / HOP_SECONDS;
			const int landed = (int)hops;
			for (int i = 0; i < landed; i++) {
				const int x = START_X + HOP_DISTANCE * (i + 1) + 4;
				drawEgg(f, x, i, loopTime - (i + 1) * HOP_SECONDS, fade);
			}

			// Get the place of the bunny: it crouches, then jumps forward along an arc
			const float phase = hops - landed;
			const float air = constrain((phase - REST) / (1.0f - REST), 0.0f, 1.0f);
			const int x = (int)lroundf(START_X + HOP_DISTANCE * (landed + air));
			const int lift = (int)lroundf(sinf(PI * air) * HOP_LIFT);
			const bool isFlying = air > 0.0f && air < 1.0f;

			// Draw the bunny
			drawBunny(f, x, 1 - lift, isFlying);

			// Draw the tall blades of grass in front of the eggs and the bunny's feet
			for (int px = 0; px < WIDTH; px++) {
				const int hash = (px * 7 + 3) % 5;
				if (hash < 3) {
					setPixel(f, px, 9, hash == 0 ? Color{ 40, 150, 50 } : Color{ 22, 110, 36 });
				}
				if (px % 11 == 4) {
					setPixel(f, px, 8, { 40, 150, 50 });
				}
			}
		}

	private:
		static constexpr float LOOP_SECONDS = 14.0f;
		static constexpr float FADE_SECONDS = 1.2f; // Fade of the eggs at the end of the loop
		static constexpr float HOP_SECONDS = 1.2f; // Time of a whole hop, crouch included
		static constexpr float REST = 0.25f; // Share of the hop spent crouching on the ground
		static constexpr int HOP_DISTANCE = 8; // Columns covered by a hop
		static constexpr float HOP_LIFT = 2.0f; // Rows the bunny rises at the top of a hop
		static constexpr int START_X = -10; // Column of the bunny's tail when the loop starts (it starts off the matrix)
		static constexpr Color EGGS[][2] = { { { 255, 110, 170 }, { 255, 240, 200 } }, { { 110, 190, 255 }, { 255, 230, 80 } }, { { 190, 130, 255 }, { 255, 255, 255 } }, { { 255, 200, 60 }, { 255, 90, 140 } }, { { 90, 230, 160 }, { 255, 255, 255 } }, { { 255, 140, 90 }, { 120, 200, 255 } } }; // Base and stripe colors of the eggs
		static constexpr int EGG_COLOR_COUNT = sizeof(EGGS) / sizeof(EGGS[0]); // Number of egg color pairs

		// The bunny facing right, standing on the ground (feet on the last row), then jumping (w body, p pink, k eye)
		static constexpr const char *BUNNY_BODY[] = {
			"......wp.pw",
			"......wp.pw",
			"......wp.pw",
			"......wwwww",
			"..wwwwwwkww",
			".wwwwwwwwwp",
			"wwwwwwwwww.",
			"wwwwwwwww..",
		};
		static constexpr const char *BUNNY_STANDING = ".ww..ww....";
		static constexpr const char *BUNNY_JUMPING = "ww.......ww";

		float loopTime = 0; // Seconds into the loop
		int colorShift = 0; // Which egg colors come first in this loop

		// Draw the sky from deep blue at the top to a soft mauve at the horizon, with a pale sun
		static void drawSky(Frame &f, float t) {
			for (int y = 0; y < 10; y++) {
				const Color row = blendColor({ 8, 10, 38 }, { 70, 48, 90 }, y / 9.0f);
				for (int x = 0; x < WIDTH; x++) {
					f[y][x] = row;
				}
			}

			// Draw the sun with a warm halo
			const float glow = 0.8f + 0.2f * sinf(t * 1.5f);
			for (int dy = -1; dy <= 1; dy++) {
				for (int dx = -1; dx <= 1; dx++) {
					setPixel(f, 62 + dx, 3 + dy, scaleColor({ 255, 190, 60 }, (dx == 0 && dy == 0 ? 0.9f : 0.35f) * glow));
				}
			}
		}

		// Get the color of a bunny sprite character
		static Color bunnyColor(char c) {
			switch (c) {
				case 'p':
					return { 255, 120, 170 };
				case 'k':
					return { 20, 10, 30 };
				default:
					return { 235, 225, 235 };
			}
		}

		// Draw the bunny with its top at y, with the legs stretched while it flies
		static void drawBunny(Frame &f, int x, int y, bool isFlying) {
			for (int r = 0; r < 9; r++) {
				const char *row = r < 8 ? BUNNY_BODY[r] : (isFlying ? BUNNY_JUMPING : BUNNY_STANDING);
				for (int c = 0; row[c]; c++) {
					if (row[c] != '.') {
						setPixel(f, x + c, y + r, bunnyColor(row[c]));
					}
				}
			}
		}

		// Draw a painted egg of 4 by 4 pixels around x, popping in with a sparkle (age is the time since it was dropped)
		void drawEgg(Frame &f, int x, int index, float age, float fade) const {
			const Color base = scaleColor(EGGS[(index + colorShift) % EGG_COLOR_COUNT][0], fade);
			const Color stripe = scaleColor(EGGS[(index + colorShift) % EGG_COLOR_COUNT][1], fade);
			const int style = (index + colorShift) % 3;

			// Draw the narrow top and the body
			setPixel(f, x, 6, style == 2 ? stripe : base);
			setPixel(f, x + 1, 6, style == 2 ? stripe : base);
			for (int dx = -1; dx <= 2; dx++) {
				for (int y = 7; y <= 9; y++) {
					setPixel(f, x + dx, y, base);
				}
			}

			// Paint the stripe: a band, a row of dots or a zigzag
			for (int dx = -1; dx <= 2; dx++) {
				if (style == 0) {
					setPixel(f, x + dx, 7, stripe);
				} else if (style == 1) {
					setPixel(f, x + dx, 7 + (dx & 1), stripe);
				}
			}

			// Sparkle for a moment after the drop
			if (age < 0.4f) {
				addPixel(f, x + (int)(age * 10) % 3 - 1, 5 - (int)(age * 5), scaleColor(WHITE, 1.0f - age * 2.5f));
			}
		}
	};

} // namespace

// Get the scene (created on first use)
Scene &getBunnyScene() {
	static BunnyScene scene;
	return scene;
}
