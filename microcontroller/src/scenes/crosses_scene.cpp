#include "scene_list.h"

#include "scenes/effects.h"

namespace {

	// Placeholder, drawn for real later
	class CrossesScene : public Scene {
	public:
		// Get the id and the name of the scene
		const char *id() const override { return "crosses"; }
		const char *name() const override { return "Three crosses"; }

		void begin() override {}

		void draw(Frame &f, float t, float dt) override {
			fillFrame(f, { 0, 0, 0 });
		}
	};

} // namespace

// Get the scene (created on first use)
Scene &getCrossesScene() {
	static CrossesScene scene;
	return scene;
}
