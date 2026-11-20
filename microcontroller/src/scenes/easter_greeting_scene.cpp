#include "scene_list.h"

#include "scenes/effects.h"

namespace {

	// Placeholder, drawn for real later
	class EasterGreetingScene : public Scene {
	public:
		// Get the id and the name of the scene
		const char *id() const override { return "easter-greeting"; }
		const char *name() const override { return "Greeting"; }

		void begin() override {}

		void draw(Frame &f, float t, float dt) override {
			fillFrame(f, { 0, 0, 0 });
		}
	};

} // namespace

// Get the scene (created on first use)
Scene &getEasterGreetingScene() {
	static EasterGreetingScene scene;
	return scene;
}
