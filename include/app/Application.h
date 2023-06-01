#pragma once

#include "app/Scene.h"
#include "app/Console.h"

#include "Windowing.h"
#include "Audio.h"
#include "Input.h"

class Application
{
public:
	Application();
	~Application();

	Scene CreateScene();
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