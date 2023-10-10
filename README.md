# Winter Framework

Main goal of this project is to simplify what I have been doing in IwEngine. Before I had a folder with 10+ files for each of the major systems. I didn't use 
many libraries in IwEngine because my goal was to learn how to create them, and now I see how much work it takes to harden and maintain libraries 
like EnTT and Box2d. Therefore, in this project I make full use of as many libraries as possible and mainly focus on tying them together and making simple & clean APIs.

Focusing on making it as small and understandable as possible. I have a feeling that each of the libraries from before could just be a single file.
Most of the functionality gets included no matter what, so a single file makes it simpler to use. Also, a single file lets you know the level of abstraction 
as you get deeper into the file as things need to be defined before they can be used. Looking at a folder of many files does not give you this info at a glance.

| File | Description |
| --- | --- |
| Audio | A wrapper around FMOD exposing the API through components. |
| Clock | Tracks frame time and fixed time in global variables. |
| Entity | A wrapper around EnTT exposing the API through an entity class. |
| Event | An events system based on composing std::functions to member functions. |
| Input | Maps from the window message pump to user defined axes. |
| Log | A simple logging library that can filter on severity and type. Styles can be customized through the use of tags at the start of the log. |
| Physics | A wrapper around box2d that ties into the entity system. |
| Rendering | Taking ideas from CUDA about host and device memory, this follows the same naming scheme through a unified interface for each OpenGL object. Currently supports Texture/Target, Buffer/Mesh and ShaderProgram |
| Windowing | Uses SDL to create a window and pump messages. Translates them into framework events and sends them to the main event bus |
| Folder | |
| app | The main loop and scene system |
| ext | Extensions that make use of the framework |
| util | Simple data structures and other classes used by the framework |
| v2 | The next versions of each of the above. |

# Build

```
git clone https://github.com/IainWinter/WinterFramework
cd WinterFramework
mkdir build
cd build
cmake ../
```
then use ```make``` or Visual Studio

# Examples

This is the simplest app you can make

```c++
#include "app/EngineLoop.h"

int main()
{
	RunEngineLoop<EngineLoopBase>();
}
```

Here is an example of an app that draws a red square on the screen. It can be moved around with WASD and a connected controller's left joystick. It's currently in a bad state because I am moving between v1 and v2 of some of the API.

```c++
#include "app/EngineLoop.h"
#include "app/Update.h"
#include "ext/rendering/BatchSpriteRenderer.h"
#include "Clock.h"

#include "v2/EntitySystem.h"
#include "v2/Render/Camera.h"
#include "v2/Render/CameraAdapter.h"

class ExampleSystem : public SystemBase {
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

class ExampleLoop : public EngineLoop<ExampleLoop> {
public:
	void _Init() override {
		// Scenes hold all entity data & systems for updating state
		Scene scene = app.CreateScene(new v2EntitySceneData());
		
		scene.CreateGroup()
			.Then<ExampleSystem>();

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
	RunEngineLoop<ExampleLoop>();
}
```
