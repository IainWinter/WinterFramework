#pragma once

#include "Windowing.h"
#include "Entity.h"
#include "Physics.h"
#include "Audio.h"
#include "ext/Task.h"
#include "ext/Time.h" // bring this in for Delay

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

private:
	template<typename _t>
	friend struct System;

// public interface

public:

	// some of these do extra stuff, but I like the idea of having the ability to
	// put the metrics here instead of in the location where they are called

	void _Init(World* world); // pass world so every system doestn need a constructor
	void _Dnit();

	void _Activate();
	void _Deactivate();

	void _Update();
	void _FixedUpdate();

	void _UI();
	void _Debug();

	virtual ~SystemBase() = default;

protected:

// creating entities

	Entity CreateEntity();
	Entity WrapEntity(u32 entityId);

// entity queries

	template<typename... _t> EntityQuery<_t...>           Query();
	template<typename... _t> EntityQueryWithEntity<_t...> QueryWithEntity();
	template<typename... _t> Entity                       FirstEntity();
	template<typename    _t> _t&                          First();

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

	int Chirp(const std::string& audioEvent);

// getters

	WindowRef GetWindow();
	World*    GetWorld();
// interface

protected:
	virtual void Init() {} // alloc resources
	virtual void Dnit() {} // free resources

	virtual void Activate() {}   // attach events
	virtual void Deactivate() {} // detach events, happens automatically in _Detach

	virtual void Update() {}
	virtual void FixedUpdate() {}

	virtual void UI() {}
	virtual void Debug() {}
};

template<typename _t>
struct System : SystemBase
{
	template<typename _e>
	void Attach();

	template<typename _e>
	void Detach();
};

struct World final
{
private:
	Application* m_app;

	// before the design was to store everything in the entity system
	// I think this might be the way to do as it would allow worlds to choose to add
	// physics / audio

	// another argument is that including these and not using them doesnt hurt anything, so
	// its uo in the air...

	// the way to do this would be to make a root entity world in the application
	// then store worlds in that

	// then the World has its own EntityWorld that can store the physics / audio in a component

	EntityWorld  m_entities;
	PhysicsWorld m_physics;
	AudioWorld   m_audio;
	TaskPool     m_tasks;

	event_manager m_bus;
	event_queue   m_queue;

	std::vector<SystemBase*> m_systems;

	// tick variables
	float m_fixedTimeAcc = 0.f;
	bool m_active = true;

public:
	void SetActive(bool active);
	bool IsActive() const;

public:
	Application*   GetApplication();
	EntityWorld&   GetEntityWorld();
	PhysicsWorld&  GetPhysicsWorld();
	AudioWorld&    GetAudioWorld();
	TaskPool&      GetTaskPool();
	event_queue&   GetEventQueue();
	event_manager& GetEventBus();
	WindowRef      GetWindow();

public:
	void Tick();
	void TickUI();

private:
	void TickFixed();
	void TickSystems();
	void TickSystemsUI();
	void TickSystemsDebug();
	void TickTasks();
	void TickEvents();
	void TickEntities();
	void TickAudio();

public:
	World(Application* app, event_manager* root);
	~World();
	
	void DetachFromRoot(event_manager* root);

	template<typename _t>
	SystemBase* CreateSystem(_t&& system);

	void  AttachSystem(SystemBase* system);
	void  DetachSystem(SystemBase* system);
	void DestroySystem(SystemBase* system);

private:
	// needs to use entt because they dont support lambdas 

	void on_add_rigidbody(entt::registry& reg, entt::entity e);
	void on_remove_rigidbody(entt::registry& reg, entt::entity e);
};

struct Application final
{
private:
	Window m_window;

	event_manager m_bus;
	event_queue   m_queue;

	std::vector<World*> m_worlds;

public:
	Window&        GetWindow();
	event_manager& GetRootEventBus();
	event_queue&   GetRootEventQueue();

public:
	Application();

	World* CreateWorld();
	void DestroyWorld(World* world);

	void Tick();

// attaching and sending to root event queue

	template<typename _e, typename _h> void Attach (_h* handler);
	template<typename _e>              void Detach (void* handler);
	                                   void Detach (void* handler);
	template<typename _e>              void Send   (_e&& event);
	template<typename _e>              void SendNow(_e&& event);
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

template<typename... _t> 
inline Entity SystemBase::FirstEntity()
{
	EntityQueryWithEntity<_t...> query = QueryWithEntity<_t...>();
	
	if (query.begin() == query.end())
	{
		return Entity();
	}

	return std::get<0>(*query.begin());
}

template<typename _t> 
inline _t& SystemBase::First()
{
	EntityQuery<_t> query = Query<_t>();
	
	if (query.begin() == query.end())
	{
		printf("[ERROR] First failed, no entity exists with component");
		throw nullptr;
	}

	return std::get<0>(*query.begin());
}

template<typename _e>
inline void SystemBase::Send(_e&& event)
{
	m_world->GetEventQueue().send(std::forward<_e>(event));
}

template<typename _e>
inline void SystemBase::SendNow(_e&& event)
{
	m_world->GetEventBus().send(std::forward<_e>(event));
}

template<typename _e>
inline void SystemBase::SendToRoot(_e&& event)
{
	m_world->GetApplication()->GetRootEventQueue().send(std::forward<_e>(event));
}

template<typename _e>
inline void SystemBase::SendToRootNow(_e&& event)
{
	m_world->GetApplication()->GetRootEventBus().send(std::forward<_e>(event));
}

template<typename _t>
template<typename _e>
inline void System<_t>::Attach()
{
	m_world->GetEventBus().attach<_e, _t>((_t*)this);
}

template<typename _t>
template<typename _e>
inline void System<_t>::Detach()
{
	m_world->GetEventBus().detach<_e>(this);
}

template<typename _t>
inline SystemBase* World::CreateSystem(_t&& system)
{
	SystemBase* s = new _t(std::forward<_t>(system));
	s->_Init(this);
	AttachSystem(s);

	return s;
}

template<typename _e, typename _h> 
inline void Application::Attach(_h* handler)
{
	m_bus.attach<_e, _h>(handler);
}

template<typename _e> 
inline void Application::Detach(void* handler)
{
	m_bus.detach<_e>(handler);
}

inline void Application::Detach(void* handler)
{
	m_bus.detach(handler);
}

template<typename _e> 
inline void Application::Send(_e&& event)
{
	m_queue.send<_e>(std::forward<_e>(event));
}

template<typename _e> 
inline void Application::SendNow(_e&& event)
{
	m_bus.send<_e>(std::forward<_e>(event));
}



































//
//
//
//
//
//
//// this kinda sucks
//// problem with having two levels loaded at once
//
//// This regolith game needs to be like this
//
//// Main Menu UI -> UI / GAME
//// BACKGROUND   -> SAME BACKGROUND
//
//// so like the layer system from before, but I didnt like how complex that was and the events being bubbled through sucked
//
//
//#include "Common.h"
//#include "Entity.h"
//#include "Event.h"
//#include "ext/Task.h"
//#include "util/metrics.h" // beings time into Leveling.h, before this it wasnt needed...
//						  // should bring time into the main lib though, no reason for it to be an ext
//
//// might want to change the name of this file, it is a little confusing to include
//
//// This holds the global state for the program
//// each module represents a plugin / major piece of the framework
//
//// An example would be
//
//// 1. Window / Rendering context
//// 2. Physics World
//// 3. Audio Engine
//// 4. Sand World
//// 5. Sprite Renderer
//// 6. Triangle Renderer
//// 7. Level Manager
//
//// This allows the user to enable / disable the functionality they need
//
//struct Application
//{
//private:
//	Entity m_modules;
//	EntityWorld m_root;
//
//	event_manager m_rootBus; // all engine events + inter level comms
//	event_queue m_rootQueue;
//	
//	TaskPool m_task; // one thread pool per app makes the most sense
//
//	metrics_log m_metrics;
//
//public:
//	Application()
//		: m_rootQueue (&m_rootBus)
//	{
//		 m_modules = m_root.Create(); 
//	}
//
//	~Application()
//	{
//		m_modules.Destroy();
//	}
//
//	template<typename    _t> _t&                    GetModule()                { return m_modules.Get<_t>(); }
//	template<typename... _t> std::tuple<_t&...>     GetModules()               { return m_modules.GetAll<_t...>(); }
//	template<typename    _t> void                   RemoveModule()             { m_modules.Remove<_t>(); }
//	template<typename    _t, typename... _args> _t& AddModule(_args&&... args) { return m_modules.Add<_t>(std::forward<_args>(args)...); }
//
//	// window needs this, but annoying to have to expose like this
//	// or maybe these are the only two access functions
//	// and the below are removed
//
//	event_queue* GetRootEventQueue() { return &m_rootQueue; }
//	TaskPool*    GetTaskPool()       { return &m_task; }
//
//	// Debugging metrics
//
//	scope_timer  TimeScope (const char* name) { return scope_timer(name, &m_metrics); };
//	metrics_log* GetMetrics()                 { return &m_metrics; }
//
//	// subject to removal ? \/
//
//	// access to the root queue
//	// ideally these wouldnt be here and everything would be through systems
//
//	template<typename _t, typename _h> void Attach (_h*   handler) { m_rootBus.attach<_t, _h>(handler); }
//	template<typename _t>              void Detach (void* handler) { m_rootBus.detach<_t>(handler); }
//	                                   void Detach (void* handler) { m_rootBus.detach(handler); }
//	template<typename _t>              void Send   (_t&& event)    { m_rootQueue.send(event); }
//	template<typename _t>              void SendNow(_t&& event)    { m_rootBus.send(event); }
//};
//
//struct Level;
//struct SystemBase;
//template<typename _me>
//struct System;
//
//struct Level
//{
//private:
//	int m_levelId; // not really needed was for global ecs to mark entities
//	Application* m_app;
//	
//	event_manager m_levelBus; // intra level comms
//	event_queue m_levelQueue; // intra level comms
//
//	EntityWorld m_world;
//	std::vector<SystemBase*> m_systems; // updater functions
//
//	bool m_initialized = false;
//
//	template<typename _t> friend struct System;
//	                      friend struct SystemBase;
//
//public:
//	Level(int levelId, Application* owning)
//		: m_levelId    (levelId)
//		, m_app        (owning)
//		, m_levelQueue (&m_levelBus)
//	{}
//	
//	~Level(); // calls SystemBase::~SystemBase
//
//	// EngineLoop needs GetLevelEventQueue, but annoying to have to expose like this
//
//	int                             Id()                const { return m_levelId; }
//	const std::vector<SystemBase*>& GetSystems()        const { return m_systems; }
//	Entity                          CreateEntity()            { return m_world.Create(); }
//
//	// these should be removed to force use of systems for updating
//
//	EntityWorld*                    GetWorld()                { return &m_world; }
//	Application*                    GetApp()                  { return m_app; }
//	event_queue*                    GetLevelEventQueue()      { return &m_levelQueue; }
//	
//	// system constructors should be only default init of values
//	// wait until Start to do any work
//
//	template<typename _t>
//	SystemBase* CreateSystem(_t&& systemToMove)
//	{
//		SystemBase* system = NewSystem<_t>(std::forward<_t>(systemToMove));
//		m_systems.push_back(system);
//		return system;
//	}
//
//	void DestroySystem(SystemBase* system)
//	{
//		auto itr = std::find(m_systems.begin(), m_systems.end(), system);
//		m_systems.erase(itr);
//		DeleteSystem(system);
//	}
//
//	void DestroySystems(const std::vector<SystemBase*>& systems)
//	{
//		for (SystemBase* system : systems) DestroySystem(system);
//	}
//
//private:
//	template<typename _t>
//	SystemBase* NewSystem(_t&& system_toCopy);
//	
//	void DeleteSystem(SystemBase* system);
//};
//
//struct SystemBase
//{
//private:
//	Level* m_level = nullptr; // get set in Level::NewSystem
//	const char* m_name = nullptr; // ONLY for metrics, going to use an function in Init, not force through a constructor
//
//	template<typename _t> friend struct System;
//	                      friend struct Level;
//
//protected:
//	template<typename... _t> EntityQuery<_t...>           Query()           { assert_init(); return m_level->m_world.Query<_t...>(); }
//	template<typename... _t> EntityQueryWithEntity<_t...> QueryWithEntity() { assert_init(); return m_level->m_world.QueryWithEntity<_t...>(); }
//	template<typename    _t> _t&                          GetModule()       { assert_init(); return m_level->m_app->GetModule<_t>(); }
//	template<typename... _t> std::tuple<_t&...>           GetModules()      { assert_init(); return m_level->m_app->GetModules<_t...>(); }
//
//// some helpers
//
//	template<typename... _t> 
//	Entity FirstEntityWith() const
//	{
//		assert_init(); 
//
//		auto query = m_level->m_world.QueryWithEntity<_t...>();
//		if (query.begin() == query.end())
//		{
//			return Entity();
//		}
//
//		return std::get<0>(*m_level->m_world.QueryWithEntity<_t...>().begin());
//	}
//
//	template<typename _t> void Send         (_t&& event) { assert_init(); m_level->m_levelQueue.send(event); }
//	template<typename _t> void SendNow      (_t&& event) { assert_init(); m_level->m_levelBus.send(event); }
//	template<typename _t> void SendToRoot   (_t&& event) { assert_init(); m_level->m_app->Send(event); }
//	template<typename _t> void SendToRootNow(_t&& event) { assert_init(); m_level->m_app->SendNow(event); }
//
//// Entities
//
//	Entity CreateEntity() { assert_init(); return m_level->CreateEntity(); }
//	Entity Wrap(u32 id)   { assert_init(); return m_level->GetWorld()->Wrap(id); }
//
//// Threading
//	
//	void Thread   (const std::function<void()>& work) { assert_init(); m_level->m_app->GetTaskPool()->Thread(work); }
//	void Coroutine(const std::function<bool()>& work) { assert_init(); m_level->m_app->GetTaskPool()->Coroutine(work); }
//	void Defer    (const std::function<void()>& work) { assert_init(); m_level->m_app->GetTaskPool()->Defer(work); }
//
//// Instead of accessing global LevelManager::CurrentLevel
//
//	Level* GetLevel() { return m_level; }
//
//// Debugging metrics
//
//	scope_timer TimeScope(const char* name) { return m_level->m_app->TimeScope(name); };
//
//// Metadata
//
//public:
//	void        SetName(const char* name) { m_name = name; }
//	const char* GetName() { return m_name; }
//
//// Interface
//
//public:
//	virtual ~SystemBase() {}
//
//	// I wonder if you could just declspec a templated type to
//	// test if it contained these functions and store their func pointers if they do
//	// the last engine got bogged down with excessive empty virtual functions
//
//	virtual void Init() {}
//	virtual void Dnit() {}
//
//	virtual void Update() {}
//	virtual void FixedUpdate() {}
//	virtual void UI() {}
//	virtual void Debug() {}
//
//private:
//	void assert_init() const
//	{
//		assert(m_level && "System has not been assigned a level. Make sure to use the Init function, not the constructor");
//	}
//};
//
//// attach needs to know the concrete type
//// this also allows for the above to happen with decl
//
//// allows for attaching events
//template<typename _me>
//struct System : SystemBase
//{
//	template<typename _t> void Attach() { assert_init(); m_level->m_levelBus.attach<_t, _me>((_me*)this); }
//	template<typename _t> void Detach() { assert_init(); m_level->m_levelBus.detach<_t>(this); }
//};
//
//// manages subletting the ECS for moving between levels
//// this will be useful for moving between main menu / game play without needing to delete everything yourself
//
//struct LevelManager
//{
//private:
//	int m_nextLevelId;
//	Application* m_app;
//
//	std::unordered_map<int, r<Level>> m_levels;
//	inline static r<Level> m_current;
//
//public:
//	LevelManager(Application& owning)
//		: m_nextLevelId (1)
//		, m_app         (&owning)
//	{}
//
//	// dont worry about move / copies...
//	void Destroy()
//	{
//		while (m_levels.size() > 0)
//		{
//			DestroyLevel(m_levels.begin()->first);
//		}
//	}
//
//	// Creating levels
//	// this is where they would be loaded form file
//	// that would call CreateLevel and add all its Systems then call InitLevel
//	// for now we can do that manually
//
//	r<Level> CreateLevel()
//	{
//		r<Level> level = mkr<Level>(m_nextLevelId, m_app);
//		m_levels.emplace(m_nextLevelId, level);
//		m_nextLevelId += 1;
//
//		if (!m_current) m_current = level;
//
//		m_app->GetRootEventQueue()->m_manager->attach_child(level->GetLevelEventQueue()->m_manager);
//
//		return level;
//	}
//
//	void DestroyLevel(int levelId)
//	{
//		r<Level> level = m_levels.at(levelId);
//		//DnitLevel(level);
//
//		if (m_current == level)
//		{
//			m_current = nullptr;
//		}
//
//		m_levels.erase(levelId);
//		m_app->GetRootEventQueue()->m_manager->detach_child(level->GetLevelEventQueue()->m_manager);
//	}
//
//	//void InitLevel(r<Level> level)
//	//{
//	//	for (SystemBase* system : level->GetSystems())
//	//	{
//	//		system->Init();
//	//	}
//	//}
//
//	//void DnitLevel(r<Level> level)
//	//{
//	//	for (SystemBase* system : level->GetSystems())
//	//	{
//	//		system->Dnit();
//	//	}
//	//}
//
//	static r<Level> CurrentLevel()
//	{
//		return m_current;
//	}
//};
//
//// Level Manager
//
//inline static r<Level> m_current = nullptr;
//
//// Level
//
//inline Level::~Level()
//{
//	for (SystemBase* system : m_systems) delete system;
//}
//
//template<typename _t>
//inline SystemBase* Level::NewSystem(_t&& system_toCopy)
//{
//	SystemBase* system = new _t(std::forward<_t>(system_toCopy));
//	system->m_level = this;
//	system->SetName(typeid(_t).name()); // default name, can set a pritty name in the Init function of system
//	system->Init();
//	return system;
//}
//
//inline void Level::DeleteSystem(SystemBase* system)
//{
//	m_levelBus.detach(system);
//	system->Dnit();
//	delete system;
//}

