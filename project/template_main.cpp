#include "app/EngineLoop.h"
#include "app/Update.h"
#include "ext/rendering/BatchSpriteRenderer.h"
#include "Clock.h"

#include "v2/EntitySystem.h"
#include "v2/Render/Camera.h"
#include "v2/Render/CameraAdapter.h"

class PROJECT_NAMESystem : public SystemBase {
public:
	BatchSpriteRenderer render;
	vec2 redSquarePos = vec2(0.f);

	void Update() override {
		redSquarePos += GetAxis("Move") * 5.f * Time::DeltaTime();

		Transform2D redSquareTransform = Transform2D(redSquarePos);
		Camera camera = FromCameraLens(lens_Orthographic(5, Render::GetWindowAspect(), -1, 1));

		render.Begin();
		render.SubmitColor(redSquareTransform, Color(255, 10, 10));
		render.Draw(camera);
	}
};

class PROJECT_NAMELoop : public EngineLoop<PROJECT_NAMELoop> {
public:
	void _Init() override {
		// Scenes hold all entity data & systems for updating state
		Scene scene = app.CreateScene(new v2EntitySceneData());
		
		scene.CreateGroup()
			.Then<PROJECT_NAMESystem>();

		// Binding keyboard and controller inputs to axes

		// Create an axis for the left joystick 
		app.input.CreateAxis("Left Stick")
			.Map(cAXIS_LEFTX, vec2(1.f, 0.f))
			.Map(cAXIS_LEFTY, vec2(0.f, 1.f))
			.Deadzone(0.1f);
		
		// Create an axis for the WASD keys
		app.input.CreateAxis("WASD")
			.Map(KEY_W, vec2( 0.f,  1.f))
			.Map(KEY_A, vec2(-1.f,  0.f))
			.Map(KEY_S, vec2( 0.f, -1.f))
			.Map(KEY_D, vec2( 1.f,  0.f));

		// Combine these axes into a single group axis
		app.input.CreateGroupAxis("Move")
			.Map("Left Stick")
			.Map("WASD");

		// Attaching events to the main bus
		Attach<event_Input>();
	}

	// events aren't virtual and get attached by instance 
	void on(event_Input& e) {
		log_game("%s: (%2.2f, %2.2f)", e.name.c_str(), e.axis.x, e.axis.y);
	}
};

// no bootstrapped main function

int main() {
	RunEngineLoop<PROJECT_NAMELoop>();
}