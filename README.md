Main goal of this project is to simplify what I have been doing in IwEngine. Before I had a folder with 10+ files for each of the major systems. I didn't use many libraries in IwEngine because my goal was to learn how to create them, and now I see how much work it takes to harden and maintain libraries like EnTT and Box2d. Therefore, in this project I make full use of as many libraries as possible and mainly focus on tying them together and making simple & clean APIs.

Focusing on making it as small and understandable as possible. I have a feeling that each of the libraries from before could just be a single file.
Most of the functionality gets included no matter what, so a single file makes it simpler to use. Also, a single file lets you know the level of abstraction as you get deeper into the file as things need to be defined before they can be used. Looking at a folder of many files does not give you this info at a glance.

| Piece of framework | Description |
| --- | --- |
| Audio | An interface to talk to underlying audio engines through handles. Currently implements FMOD |
| Entity | A wrapper around EnTT exposing the API through an entity class |
| Event | An events system based on composing std::functions to member functions. |
| Physics | A wrapper around box2d that ties into the entity system to automatically maintain the physics world |
| Rendering | Taking ideas from CUDA about host and device memory, this follows the same naming scheme through a unified interface for each OpenGL object. Currently supports Texture/Target, Buffer/Mesh and ShaderProgram |
| Windowing | Uses SDL to create a window and pump messages. Translates them into framework events and sends them to the main event bus |
| App | Ties everything together to create an application framework. Can load Worlds and attach Systems to update the state. Each World stores its own entities, and multiple can be loaded at once to compose scenes. |

# Examples

This is the simplest app you can make

```c++
#include "app/EngineLoop.h"

int main()
{
	RunEngineLoop<EngineLoopBase>();
}
```

Here is an example of an app that draws a red square on the screen. It can be moved around with WASD and a connected controller's left joystick.

```c++
#include "app/EngineLoop.h"
#include "ext/rendering/BatchSpriteRenderer.h"

struct ExampleSystem : System<ExampleSystem>
{
	BatchSpriteRenderer render;
	Entity sprite;

	void Init() override
	{
		// Creating entities in the world & adding components
		sprite = CreateEntity();
		sprite.Add<Transform2D>();
	}
	
	void Update() override
	{
		// getting components of a specific entity

		sprite.Get<Transform2D>().position += 5.f * Input::GetAxis("Move") * Time::DeltaTime();

		// use of an extension that provides a simple batch quad renderer

		render.Begin(Camera(0, 0, 10, 10));

		for (auto [transform] : Query<Transform2D>()) // querying of components from ECS
		{
			render.SubmitSprite(transform, Color(255, 0, 0));
		}

		render.Draw();
	}
};

struct Example : EngineLoop<Example>
{
	void Init() override
	{
		// Worlds hold all entity data & systems for updating state

		World* world = app.CreateWorld();
		world->CreateSystem<ExampleSystem>();

		// Binding axes to keyboard and controller inputs from SDL

		// Create an axis for the left joystick 

		Input::CreateAxis("Left Stick");
		Input::SetAxisComponent("Left Stick", cAXIS_LEFTX, vec2(1.f, 0.f));
		Input::SetAxisComponent("Left Stick", cAXIS_LEFTY, vec2(0.f, 1.f));
		Input::SetDeadzone("Left Stick", 0.1f);

		// Create an axis for the WASD keys

		Input::CreateAxis("WASD");
		Input::SetAxisComponent("WASD", SDL_SCANCODE_W, vec2( 0.f,  1.f));
		Input::SetAxisComponent("WASD", SDL_SCANCODE_A, vec2(-1.f,  0.f));
		Input::SetAxisComponent("WASD", SDL_SCANCODE_S, vec2( 0.f, -1.f));
		Input::SetAxisComponent("WASD", SDL_SCANCODE_D, vec2( 1.f,  0.f));

		// Combine these axes into a single virtual axis

		Input::CreateVirtualAxis("Move");
		Input::SetVirtualAxisComponent("Move", "Left Stick");
		Input::SetVirtualAxisComponent("Move", "WASD");

		Attach<event_Input>();
	}

	void on(event_Input& e)
	{
		log_game("%s: (%2.2f, %2.2f)", e.name, e.axis.x, e.axis.y);
	}
};

// no boostrapped main function

int main()
{
	RunEngineLoop<Example>();
}
```
