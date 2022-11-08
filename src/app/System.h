#pragma once

#include "Log.h"
#include "Windowing.h"
#include "Entity.h"
#include "Physics.h"
#include "Audio.h"

#include "app/Console.h"

#include "ext/Time.h" 
#include "ext/Task.h" // bring this in for Delay
#include "ext/rendering/DebugRender.h" // this should be switched on W_DEBUG

#include <vector>

struct SystemBase;
template<typename _t> struct System;
struct World;
struct Application;

// wanted to use entt for this id, but this works for now
using SystemId = size_t;

template<typename _t>
SystemId GetSystemId() { return typeid(_t).hash_code(); }

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
	SystemId m_id;

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

	void _SetId(SystemId id);

public:
	SystemBase();
	virtual ~SystemBase();

	bool GetInitState() const;
	bool GetActiveState() const;

	SystemId Id() const;

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

// physics

	RayQueryResult   QueryRay  (vec2 pos, vec2 end);
	RayQueryResult   QueryRay  (vec2 pos, vec2 direction, float distance);
	PointQueryResult QueryPoint(vec2 pos, float radius);

// getters

	World*        GetWorld();
	WindowRef     GetWindow();
	EntityWorld&  GetEntities();
	PhysicsWorld& GetPhysics();
	AudioWorld&   GetAudio();

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
	std::unordered_map<SystemId, SystemBase*> m_map;

	float m_fixedTimeAcc;
	const char* m_name;
	bool m_init;
	bool m_debug;

public:
	World(Application* app, EventBus* root);
	~World();
	
	void DetachFromRoot(EventBus* root);

	// will assert single system per type
	// systems should communicate through events, so these only return SystemBase*

	// create       type
	// destroy      type / instance
	// get          type
	// attach       instance
	// detach       instance
	// has          type / instance

	template<typename _t> SystemBase*  CreateSystem();
	template<typename _t> void        DestroySystem();
	template<typename _t> SystemBase*     GetSystem();
	template<typename _t> bool            HasSystem() const;

	void  AttachSystem(SystemBase* system);
	void  DetachSystem(SystemBase* system);
	void DestroySystem(SystemBase* system);
	bool     HasSystem(SystemBase* system) const;

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
	void InitUninitializedSystems();

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
	void DestroyAllWorlds();

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
	SystemId id = GetSystemId<_t>();
	auto itr = m_map.find(id);

	if (itr != m_map.end())
	{
		log_world("w~Didn't add system, already exists in world");
		return nullptr;
	}

	SystemBase* s = new _t();
	s->_SetId(id);

	m_systems.push_back(s);
	m_map.emplace(id, s);

	// actually run this in the tick loop like attachment
	//if (m_init) // auto init if world is already
	//{
	//	s->_Init(this);
	//}

	return s;
}

template<typename _t> 
inline void World::DestroySystem()
{
	SystemId id = GetSystemId<_t>();
	auto itr = m_map.find(id);

	if (itr == m_map.end())
	{
		log_world("w~Didn't destroy system, doesn't exist in world");
		return;
	}

	DestroySystem(itr->second);
}

template<typename _t>
inline SystemBase* World::GetSystem()
{
	if (!HasSystem<_t>())
	{
		log_world("w~Tried to get system that doesn't exist in world");
		return nullptr;
	}

	return m_map.find(GetSystemId<_t>())->second;
}

template<typename _t> 
inline bool World::HasSystem() const
{
	return m_map.find(GetSystemId<_t>()) != m_map.end();
}
