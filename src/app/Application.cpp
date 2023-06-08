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
	DeleteScenes();
}

Scene Application::CreateScene()
{
	SceneNode* node = new SceneNode(this);
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

void Application::DeleteScenes()
{
	for (SceneNode* s : m_scenes)
		delete s;

	m_scenes.clear();
}

void Application::Tick()
{
	// Events

	// Need to execute deferred deletes here because those may send
	// events up to the app bus which change the state in a critical way for next
	// frame

	for (SceneNode* node : m_scenes)
		node->entities.ExecuteDeferredDeletions();

	window.PumpEvents();

	bool hasEvents = false;
	do
	{
		hasEvents = false;
		hasEvents |= event.Execute();

		for (SceneNode* node : m_scenes)
			hasEvents |= node->event.Execute();

	} while (hasEvents);

	// Game tick
    
	for (SceneNode* node : m_scenes)
		node->Tick(Time::DeltaTime(), Time::FixedTime());

	// UI

	window.BeginImgui();

	for (SceneNode* node : m_scenes)
		node->TickUI();

	window.EndImgui();

	// OS
	
	audio.Tick();
	window.EndFrame();
}