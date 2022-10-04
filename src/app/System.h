#pragma once

#include "Log.h"
#include "Windowing.h"
#include "Entity.h"
#include "Physics.h"
#include "Audio.h"
#include "ext/Task.h"
#include "ext/Time.h" // bring this in for Delay

#include "app/Console.h"

#include <vector>

struct SystemBase;
template<typename _t> struct System;
struct World;
struct Application;

// System lifecycle

// constructor
// Init
// add to world
// Attach
// Detach
// remove from world
// Dnit
// release shared ptr

struct SystemBase
{
private:
	World* m_world;
	bool m_active;

private:
	template<typename _t>
	friend struct System; // for m_world without giving direct access to children

	friend struct World;  // for _Method

private:

	// some of these do extra stuff, but I like the idea of having the ability to
	// put the metrics here instead of in the location where they are called

	void _Init(World* world); // pass world so every system doestn need a constructor
	void _Dnit();

	void _OnAttach();
	void _OnDetach();

	void _Update();
	void _FixedUpdate();

	void _UI();
	void _Debug();

public:
	SystemBase();
	virtual ~SystemBase();

	bool GetInitState() const;
	bool GetActiveState() const;

protected:

// creating entities

	Entity CreateEntity();
	Entity WrapEntity(u32 entityId);

// entity queries

	template<typename... _t> EntityQuery<_t...>           Query();
	template<typename... _t> EntityQueryWithEntity<_t...> QueryWithEntity();
	template<typename    _t> _t&                          First();
	template<typename... _t> Entity                       FirstEntity();
	template<typename... _t> int                          GetNumberOf();

// sending events

	template<typename _e> void Send         (_e&& event);
	template<typename _e> void SendNow      (_e&& event);
	template<typename _e> void SendToRoot   (_e&& event);
	template<typename _e> void SendToRootNow(_e&& event);

// threading

	void Coroutine(const std::function<bool()>& task);                       // run code on main thread until func returns true
	void Thread   (const std::function<bool()>& task);                       // run code on a thread in a pool
	void Defer    (const std::function<void()>& task);                       // run code once at the end of the frame (with coroutine)
	void Delay    (float delayInSeconds, const std::function<void()>& task); // run code after x seconds

// audio

	Audio       CreateAudio   (const std::string& eventPath);
	AudioSource GetAudioSource(const std::string& eventPath);

// getters

	WindowRef   GetWindow();
	AudioWorld& GetAudio();
	World*      GetWorld();

// interface

protected:
	virtual void Init() {} // alloc resources
	virtual void Dnit() {} // free resources

	virtual void OnAttach() {} // attach events
	virtual void OnDetach() {} // detach events, happens automatically in _Detach

	virtual void Update() {}
	virtual void FixedUpdate() {}

	virtual void UI() {}
	virtual void Debug() {}
};

template<typename _t>
struct System : SystemBase
{
	virtual ~System() {}

	template<typename _e>
	void Attach();

	template<typename _e>
	void Detach();
};

struct World final
{
private:
	Application* m_app;

	EntityWorld  m_entities;
	PhysicsWorld m_physics;
	TaskPool     m_tasks;      // todo: allow this to expand and contract, right now it makes too many threads

	EventBus   m_bus;
	EventQueue m_queue;

	std::vector<SystemBase*> m_systems;

	float m_fixedTimeAcc;
	const char* m_name;
	bool m_init;
	bool m_debug;

public:
	World(Application* app, EventBus* root);
	~World();
	
	void DetachFromRoot(EventBus* root);

	// you could create two of the same system 
	// dont do that
	template<typename _t>
	SystemBase* CreateSystem();

	void  AttachSystem(SystemBase* system);
	void  DetachSystem(SystemBase* system);
	void DestroySystem(SystemBase* system);

	bool ContainsSystem(SystemBase* system) const;

	void Init();
	void SetDebug(bool debug);

public:
	Application*  GetApplication();

	// returns a WindowRef that allows editing properties of the window
	// but not its rendering state
	WindowRef     GetWindow();
	AudioWorld&   GetAudio();

	EntityWorld&  GetEntityWorld();
	PhysicsWorld& GetPhysicsWorld();
	TaskPool&     GetTaskPool();

	EventQueue&   GetEventQueue();
	EventBus&     GetEventBus();

	bool          GetInitState() const;

	const char*   GetName() const;
	World*        SetName(const char* name);

public:
	void Tick();
	void TickUI();

private:
	void TickFixed();
	void TickSystems();
	void TickSystemsUI();
	void TickSystemsDebug();

	void AttachInactiveSystems();

private:
	void AttachDefaultEntityEvents();
};

struct Application final
{
private:
	Window     m_window;
	AudioWorld m_audio;

	EventBus   m_bus;
	EventQueue m_queue;

	Console m_console;

	std::vector<World*> m_worlds;

public:
	Window&     GetWindow();
	AudioWorld& GetAudio();
	EventBus&   GetRootEventBus();
	EventQueue& GetRootEventQueue();
	Console&    GetConsole();

	const std::vector<World*>& GetWorlds() const;
	World* GetWorld(const char* name);

public:
	Application();
	~Application();

	World* CreateWorld(bool autoInit = true);
	void DestroyWorld(World* world);

	void Tick();
};

//
// template impl
//

template<typename... _t> 
inline EntityQuery<_t...> SystemBase::Query()
{
	return m_world->GetEntityWorld().Query<_t...>();
}

template<typename... _t> 
inline EntityQueryWithEntity<_t...> SystemBase::QueryWithEntity()
{
	return m_world->GetEntityWorld().QueryWithEntity<_t...>();
}

template<typename _t> 
inline _t& SystemBase::First()
{
	return m_world->GetEntityWorld().First<_t>();
}

template<typename... _t> 
inline Entity SystemBase::FirstEntity()
{
	return m_world->GetEntityWorld().FirstEntity<_t...>();
}

template<typename... _t>
inline int SystemBase::GetNumberOf()
{
	return m_world->GetEntityWorld().GetNumberOf<_t...>();
}

template<typename _e>
inline void SystemBase::Send(_e&& event)
{
	m_world->GetEventQueue().Send(std::forward<_e>(event));
}

template<typename _e>
inline void SystemBase::SendNow(_e&& event)
{
	m_world->GetEventBus().Send(std::forward<_e>(event));
}

template<typename _e>
inline void SystemBase::SendToRoot(_e&& event)
{
	m_world->GetApplication()->GetRootEventQueue().Send(std::forward<_e>(event));
}

template<typename _e>
inline void SystemBase::SendToRootNow(_e&& event)
{
	m_world->GetApplication()->GetRootEventBus().Send(std::forward<_e>(event));
}

template<typename _t>
template<typename _e>
inline void System<_t>::Attach()
{
	m_world->GetEventBus().Attach<_e, _t>((_t*)this);
}

template<typename _t>
template<typename _e>
inline void System<_t>::Detach()
{
	m_world->GetEventBus().Detach<_e>(this);
}

template<typename _t>
inline SystemBase* World::CreateSystem()
{
	SystemBase* s = new _t();
	m_systems.push_back(s);
	
	if (m_init) // auto init if world is already
	{
		s->_Init(this);
	}

	return s;
}