#pragma once

#include "Windowing.h"
#include "Audio.h"
#include "Event.h"
#include "Input.h"

#include "app/Scene.h"
#include "app/Console.h"

#include "v2/EntitySystem.h"

class Application
{
public:
	Application();
	~Application();

	Scene CreateScene(v2EntitySceneData* data); // could pass data through a function in Scene
	void DestroyScene(Scene* scene);
	void DeleteScenes();

	void Tick();

public:
	Window window;
	AudioWorld audio;
	EventBus bus;
	EventQueue event;
	Console console;
	InputMap input;

private:
	std::vector<SceneNode*> m_scenes;
};