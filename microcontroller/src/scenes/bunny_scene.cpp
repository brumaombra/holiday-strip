#include "scene_list.h"

#include "scenes/effects.h"

namespace {

	// Placeholder, drawn for real later
	class BunnyScene : public Scene {
	public:
		// Get the id and the name of the scene
		const char *id() const override { return "bunny"; }
		const char *name() const override { return "Egg hunt"; }

		void begin() override {}

		void draw(Frame &f, float t, float dt) override {
			fillFrame(f, { 0, 0, 0 });
		}
	};

} // namespace

// Get the scene (created on first use)
Scene &getBunnyScene() {
	static BunnyScene scene;
	return scene;
}
