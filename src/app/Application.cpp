#include "app/Application.h"
#include "app/Update.h"
#include "Clock.h"

Application::Application()
{
	event = EventQueue(&bus);
	window.SetEventQueue(&event);
}

Application::~Application()
{
	for (SceneNode* s : m_scenes)
		delete s;
}

Scene Application::CreateScene(const char* name)
{
	SceneNode* node = new SceneNode(this, name);
	m_scenes.push_back(node);

	// link the event buses together so Scenes get Application level events
	bus.ChildAttach(&node->bus);

	return Scene(node);
}

void Application::DestroyScene(Scene* scene)
{
	SceneNode*& node = scene->m_node;

	if (erase(m_scenes, node))
		delete node;

	node = nullptr;
}

void Application::Tick()
{
	// Events

	window.PumpEvents();
	event.Execute();

	// app loop
    
	for (SceneNode* node : m_scenes)
		node->_Tick(Time::DeltaTime(), Time::FixedTime());

	// UI

	window.BeginImgui();

	for (SceneNode* node : m_scenes)
		node->_TickUI();

	window.EndImgui();

	// OS
	
	audio.Tick();
	window.EndFrame();
}