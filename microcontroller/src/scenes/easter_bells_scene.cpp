#include "scene_list.h"

#include "scenes/effects.h"

namespace {

	// Placeholder, drawn for real later
	class EasterBellsScene : public Scene {
	public:
		// Get the id and the name of the scene
		const char *id() const override { return "easter-bells"; }
		const char *name() const override { return "Easter bells"; }

		void begin() override {}

		void draw(Frame &f, float t, float dt) override {
			fillFrame(f, { 0, 0, 0 });
		}
	};

} // namespace

// Get the scene (created on first use)
Scene &getEasterBellsScene() {
	static EasterBellsScene scene;
	return scene;
}
