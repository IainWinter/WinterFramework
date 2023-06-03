# Winter Framework



Main goal of this project is to simplify what I have been doing in IwEngine. Before I had a folder with 10+ files for each of the major systems. I didn't use many libraries in IwEngine because my goal was to learn how to create them, and now I see how much work it takes to harden and maintain libraries like EnTT and Box2d. Therefore, in this project I make full use of as many libraries as possible and mainly focus on tying them together and making simple & clean APIs.

Focusing on making it as small and understandable as possible. I have a feeling that each of the libraries from before could just be a single file.
Most of the functionality gets included no matter what, so a single file makes it simpler to use. Also, a single file lets you know the level of abstraction as you get deeper into the file as things need to be defined before they can be used. Looking at a folder of many files does not give you this info at a glance.

| File | Description |
| --- | --- |
| Audio | A wrapper around FMOD exposing the API through components. |
| Common | Some helper functions and components that get used frequently by more than a single piece of the framework. |
| Defines | Some preprocessor macros |
| Entity | A wrapper around EnTT exposing the API through an entity class |
| Event | An events system based on composing std::functions to member functions. |
| Log | A simple logging library that lets you filter on severity and type. You can also set stylings based on flags. |
| Physics | A wrapper around box2d that ties into the entity system to automatically maintain the physics world |
| Rendering | Taking ideas from CUDA about host and device memory, this follows the same naming scheme through a unified interface for each OpenGL object. Currently supports Texture/Target, Buffer/Mesh and ShaderProgram |
| Windowing | Uses SDL to create a window and pump messages. Translates them into framework events and sends them to the main event bus |
| Folder | |
| app | An Engine loop and world system that I am going to change at some point soon when I explore multithreading |
| engine | functions for passing the state of the framework over dll fences |
| ext | extensions that make use of the framework |
| util | simple data structures and other classes used by the framework  |
| io | (future) all functions that need disk/network access  |

# Build

```
git pull https://github.com/IainWinter/WinterFramework
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

Here is an example of an app that draws a red square on the screen. It can be moved around with WASD and a connected controller's left joystick.

```c++
#include "app/EngineLoop.h"
#include "ext/rendering/BatchSpriteRenderer.h"

struct ExampleSystem : SystemBase
{
	BatchSpriteRenderer render;
	Entity sprite;

	void Init() override
	{
		// Creating an entity in the world & adding components
		
		sprite = CreateEntity();
		
		sprite.Add<Transform2D>()
			.SetScale(vec2(2.f));
			
		sprite.Add<Sprite>()
			.SetTint(Color(255, 0, 0));
	}
	
	void Update() override
	{
		// getting components of a specific entity

		sprite.Get<Transform2D>().position += 5.f * GetAxis("Move") * Time::DeltaTime();

		// use of an extension that provides a simple batch quad renderer

		render.Begin();

		for (auto [transform, sprite] : Query<Transform2D, Sprite>()) // querying of components from ECS
			render.SubmitSprite(transform, sprite);

		render.Draw(Camera(10, 10, 10));
	}
};

struct Example : EngineLoop<Example>
{
	void Init() override
	{
		// Scenes hold all entity data & systems for updating state

		Scene scene = app.CreateScene();
		
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

	void on(event_Input& e) // events aren't virtual and get attached by instance
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
