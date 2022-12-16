#pragma once

#include "Log.h"
#include "Windowing.h"
#include "Entity.h"
#include "Physics.h"
#include "Audio.h"

#include "app/Console.h"
#include "app/Time.h" 
#include "app/Task.h"

#include "ext/rendering/DebugRender.h" // this should be switched on W_DEBUG
#include "ext/Transform.h"

#include "util/named.h"
#include "util/filesystem.h"

#include <vector>

struct SystemBase;
template<typename _t> struct System;
struct World;
struct Application;

// wanted to use entt for this id, but this works for now
using SystemId = size_t;

template<typename _t>
SystemId GetSystemId() { return typeid(_t).hash_code(); }

// events for dll interop

struct event_CreateEntity
{
	World* world;
	std::function<void(Entity)> callback;
};

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
// interface

protected:
	virtual void Init() {} // alloc resources
	virtual void Dnit() {} // free resources

	virtual void OnAttach() {} // attach events
	virtual void OnDetach() {} // detach events

	virtual void Update() {}
	virtual void FixedUpdate() {}

	virtual void UI() {}
	virtual void Debug() {}

private:
	r<World> m_world;
	bool m_active;
	
    SystemId m_id;
    
	std::string m_name;

private:
	template<typename _t>
	friend struct System; // for m_world without giving direct access to children

	friend struct World;  // for _Method

private:

	// some of these do extra stuff, but I like the idea of having the ability to
	// put the metrics here instead of in the location where they are called

	void _Init(r<World> world); // pass world so every system doesnt need a constructor
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
    
	SystemBase* SetName(const std::string& name);
	const char* GetName() const;

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

	r<World>      GetWorld();
	WindowRef     GetWindow();
	EntityWorld&  GetEntities();
	PhysicsWorld& GetPhysics();
	AudioWorld&   GetAudio();
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

//
//	A world is a seperated state and transformation functions that update the state
//		Each world is seperate, but it is possible to talk through events
//
//
struct World final : std::enable_shared_from_this<World>
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
	bool m_init;
	bool m_debug;

	std::string m_name;

public:
	static r<World> Make(Application* app, EventBus* root);

private:
	World(Application* app, EventBus* root);

public:
	~World();

public:
	// will assert single system per type
	// systems should communicate through events, so these only return SystemBase*

	// create       type
	// destroy      type / instance
	// get          type
	// attach       instance
	// detach       instance
	// has          type / instance

    const std::vector<SystemBase*>& GetSystems() const;
    
	template<typename _t> SystemBase*  CreateSystem();
	template<typename _t> void        DestroySystem();
	template<typename _t> SystemBase*     GetSystem();
	template<typename _t> bool            HasSystem() const;

	void  AttachSystem(SystemBase* system);
	void  DetachSystem(SystemBase* system);
	void DestroySystem(SystemBase* system);
	bool     HasSystem(SystemBase* system) const;

	void DestroyAllSystems();

	void Init();
	void SetDebug(bool debug);

	r<World> SetName(const std::string& name);
	const char* GetName() const;

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

public:
	void ReallocWorlds();
};

//
//	Attach multiple worlds at once
//

struct WorldPackage
{
	std::vector<r<World>> worlds;
};

//
//	This holds state that is shared between worlds/
//	-> Mainly the A/V system (Fmod Audio / SDL Window)
//  -> A root event bus provides communication between worlds if they bubble up events
//
//
struct Application final
{
private:
	Window     m_window;
	AudioWorld m_audio;

	EventBus   m_bus;
	EventQueue m_queue;

	Console m_console;

	std::vector<r<World>> m_worlds;

public:
	Window&     GetWindow();
	AudioWorld& GetAudio();
	EventBus&   GetRootEventBus();
	EventQueue& GetRootEventQueue();
	Console&    GetConsole();

public:
	Application();
	~Application();

	const std::vector<r<World>>& GetWorlds() const;

	// construct a new world and attach it
	r<World> CreateWorld();

	void     AttachWorld(r<World> world);
	void     DetachWorld(r<World> world);
	r<World>    GetWorld(const std::string& name) const;
	bool        HasWorld(const std::string& name) const;
	bool        HasWorld(const r<World>& world) const;

	void AttachPackage(const WorldPackage& package);
	void DetachPackage(const WorldPackage& package);

	// remove all worlds from the application update loop
	void RemoveAllWorlds();

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
