#include "scene_list.h"

#include "scenes/effects.h"

namespace {

	// Placeholder, drawn for real later
	class ChickScene : public Scene {
	public:
		// Get the id and the name of the scene
		const char *id() const override { return "chick"; }
		const char *name() const override { return "Hatching chick"; }

		void begin() override {}

		void draw(Frame &f, float t, float dt) override {
			fillFrame(f, { 0, 0, 0 });
		}
	};

} // namespace

// Get the scene (created on first use)
Scene &getChickScene() {
	static ChickScene scene;
	return scene;
}
