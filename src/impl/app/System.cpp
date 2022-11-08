#include "app/System.h"
#include "InternalSystems.h"

void SystemBase::_Init(World* world)
{
	log_world("i~\tInitialized System");

	m_world = world;
	Init();
}

void SystemBase::_Dnit()
{
	Dnit();
	m_world = nullptr;

	log_world("i~\tDeinitialized System");
}

void SystemBase::_OnAttach()
{
	log_world("i~\tAttached System");

	m_active = true;
	OnAttach();
}

void SystemBase::_OnDetach()
{
	m_active = false;
	m_world->GetEventBus().Detach(this);
	OnDetach();

	log_world("i~\tDetached System");
}

void SystemBase::_Update()
{
	Update();
}

void SystemBase::_FixedUpdate()
{
	FixedUpdate();
}

void SystemBase::_UI()
{
	UI();
}

void SystemBase::_Debug()
{
	Debug();
}

void SystemBase::_SetId(SystemId id)
{
	m_id = id;
}

SystemBase::SystemBase()
	: m_world  (nullptr)
	, m_active (false)
    , m_id     (-1)
{
	log_world("i~\tCreated System");
}

SystemBase::~SystemBase()
{
	log_world("i~\tDestroied System");
}

bool SystemBase::GetInitState() const
{
	return !!m_world;
}

bool SystemBase::GetActiveState() const
{
	return m_active;
}

SystemId SystemBase::Id() const
{
	return m_id;
}

Entity SystemBase::CreateEntity()
{
	return m_world->GetEntityWorld().Create();
}

Entity SystemBase::WrapEntity(u32 entityId)
{
	return m_world->GetEntityWorld().Wrap(entityId);
}

void SystemBase::Coroutine(const std::function<bool()>& task)
{
	return m_world->GetTaskPool().Coroutine(task);
}

void SystemBase::Thread(const std::function<bool()>& task)
{
	return m_world->GetTaskPool().Thread(task);
}

void SystemBase::Defer(const std::function<void()>& task)
{
	return m_world->GetTaskPool().Defer(task);
}

void SystemBase::Delay(float delayInSeconds, const std::function<void()>& task)
{
	float* timer = new float(delayInSeconds);
	
	auto routine = [timer, task]()
	{
		*timer -= Time::DeltaTime();
		bool done = *timer < 0.f;
		if (done)
		{
			delete timer;
			task();
		}

		return done;
	};

	return m_world->GetTaskPool().Coroutine(routine);
}

Audio SystemBase::CreateAudio(const std::string& eventPath)
{
	return GetAudio().CreateAudio(eventPath);
}

AudioSource SystemBase::GetAudioSource(const std::string& eventPath)
{
	return GetAudio().GetAudioSource(eventPath);
}

RayQueryResult SystemBase::QueryRay(vec2 pos, vec2 end)
{
	return GetWorld()->GetPhysicsWorld().QueryRay(pos, end);
}

RayQueryResult SystemBase::QueryRay(vec2 pos, vec2 direction, float distance)
{
	return GetWorld()->GetPhysicsWorld().QueryRay(pos, direction, distance);
}

PointQueryResult SystemBase::QueryPoint(vec2 pos, float radius)
{
	return GetWorld()->GetPhysicsWorld().QueryPoint(pos, radius);
}

World* SystemBase::GetWorld()
{
	return m_world;
}

WindowRef SystemBase::GetWindow()
{
	return m_world->GetWindow();
}

EntityWorld& SystemBase::GetEntities()
{
	return m_world->GetEntityWorld();
}

PhysicsWorld& SystemBase::GetPhysics()
{
	return m_world->GetPhysicsWorld();
}

AudioWorld& SystemBase::GetAudio()
{
	return m_world->GetAudio();
}

World::World(Application* app, EventBus* root)
	: m_app          (app)
	, m_queue        (&m_bus)
	, m_fixedTimeAcc (0.f)
	, m_init         (false)
	, m_debug        (false)
{
	log_world("i~Created World");

	// allow app events to propagate down to world
	root->ChildAttach(&m_bus);

	AttachDefaultEntityEvents();

	// Default system
	CreateSystem<TransformUpdate>();
	CreateSystem<PhysicsInterpolationUpdate>();
	//CreateSystem<AudioUpdate>();
}

World::~World()
{
	// wait for tasks before destroying systems
	// to keep bound lambda memory alive
	m_tasks.ShutdownAndWait();

	// should figure out what to do with events, prob execute them

	for (SystemBase* system : m_systems)
	if (system->GetInitState())
		system->_OnDetach();

	for (SystemBase* system : m_systems)
	if (system->GetInitState())
		system->_Dnit();

	for (SystemBase* system : m_systems)
		delete system;

	log_world("i~Destroied World");
}

void World::DetachFromRoot(EventBus* root)
{
	root->ChildDetach(&m_bus);
}

const std::vector<SystemBase*>& World::GetSystems() const
{
    return m_systems;
}

void World::AttachSystem(SystemBase* system)
{
	if (HasSystem(system))
	{
		log_world("e~Error: System is already in world");
		return;
	}

	m_systems.push_back(system);
	m_map.emplace(system->Id(), system);

	if (system->GetInitState())
	{
		system->_OnAttach();
	}
}

void World::DetachSystem(SystemBase* system)
{
	if (!HasSystem(system))
	{
		log_world("e~Error: System is not in world");
		return;
	}

	if (system->GetInitState())
	{
		system->_OnDetach();
	}

	m_systems.erase(std::find(m_systems.begin(), m_systems.end(), system));
	m_map.erase(system->Id());
}

void World::DestroySystem(SystemBase* system)
{
	if (!HasSystem(system)) // double error check
	{
		log_world("e~Error: System is not in world");
		return;
	}

	DetachSystem(system);
	
	if (system->GetInitState())
	{
		system->_Dnit();
	}

	delete system;
}

bool World::HasSystem(SystemBase* system) const
{
	return m_map.find(system->Id()) != m_map.end();
}

void World::Init()
{
	m_init = true;

	for (SystemBase*& system : m_systems) // init systems that need it
	{
		if (!system->GetInitState())
		{
			system->_Init(this);
		}
	}
}

void World::SetDebug(bool debug)
{
	log_world("World %s debug set to %s", GetName(), debug ? "true" : "false");
	m_debug = debug;
}

Application*  World::GetApplication()     { return m_app; }
WindowRef     World::GetWindow()          { return WindowRef(&m_app->GetWindow()); }
AudioWorld&   World::GetAudio()           { return m_app->GetAudio(); }

EntityWorld&  World::GetEntityWorld()     { return m_entities; }
PhysicsWorld& World::GetPhysicsWorld()    { return m_physics; }
TaskPool&     World::GetTaskPool()        { return m_tasks; }

EventBus&     World::GetEventBus()        { return m_bus; }
EventQueue&   World::GetEventQueue()      { return m_queue; }

bool          World::GetInitState() const { return m_init; }

void World::Tick()
{
    Debug::Begin();
    
	InitUninitializedSystems();
	AttachInactiveSystems();

	m_fixedTimeAcc += Time::DeltaTime();
	if (m_fixedTimeAcc >= Time::FixedTime())
	{
		m_fixedTimeAcc = 0.f; // should subtract the fixed time, but that cuases runaway frame drops
		TickFixed();
	}

	TickSystems();

	m_tasks.TickCoroutines();
	m_queue.Execute();
	m_entities.ExecuteDeferdDeletions();
    
	if (m_entities.GetNumberOf<Camera>() > 0) // this kinda sucks
	{
		Debug::End(m_entities.First<Camera>()); // get scene props component
	}
}

void World::TickUI()
{
	TickSystemsUI();
	
	if (m_debug)
	{
		TickSystemsDebug();
	}
}

void World::TickFixed()
{
	for (SystemBase*& system : m_systems)
	{
		system->_FixedUpdate();
	}

	m_physics.Tick(Time::FixedTime());
}

void World::TickSystems()
{
	for (SystemBase*& system : m_systems)
	{
		system->_Update();
	}
}

void World::TickSystemsUI()
{
	for (SystemBase*& system : m_systems)
	{
		system->_UI();
	}
}

void World::TickSystemsDebug()
{
	for (SystemBase*& system : m_systems)
	{
		system->_Debug();
	}
}

// could store a list of to be init / activate

void World::AttachInactiveSystems()
{
	for (SystemBase*& system : m_systems)
	{
		if (!system->GetActiveState())
		{
			system->_OnAttach();
		}
	}
}

void World::InitUninitializedSystems()
{
	for (SystemBase*& system : m_systems)
	{
		if (!system->GetInitState())
		{
			system->_Init(this);
		}
	}
}

void World::AttachDefaultEntityEvents()
{
	// auto add/remove to physics world

	m_entities.OnAdd   <Rigidbody2D>([this](Entity e) { m_physics.Add(e); });
	m_entities.OnRemove<Rigidbody2D>([this](Entity e) { m_physics.Remove(e); });
}

//void World::on_add_rigidbody(entt::registry& reg, entt::entity e)
//{
//	Entity entity = m_entities.Wrap((u32)e);
//	m_physics.Add(entity);
//}
//
//void World::on_remove_rigidbody(entt::registry& reg, entt::entity e)
//{
//	Entity entity = m_entities.Wrap((u32)e);
//	m_physics.Remove(entity);
//}

//void World::on_add_audio_emitter(entt::registry& reg, entt::entity e)
//{
//	Entity entity = m_entities.Wrap((u32)e);
//	entity.Get<AudioEmitter>()._SetAudio(&GetApplication()->GetAudio());
//}

Application::Application()
	: m_queue  (&m_bus)
	, m_window (WindowConfig{}, &m_queue)
{}

Application::~Application()
{
	DestroyAllWorlds();
}

World* Application::CreateWorld(bool autoInit)
{
	World* world = new World(this, &m_bus);
	m_worlds.push_back(world);
	
	if (autoInit)
	{
		world->Init();
	}

	return world;
}

void Application::DestroyWorld(World* world)
{
	m_worlds.erase(std::find(m_worlds.begin(), m_worlds.end(), world));
	world->DetachFromRoot(&m_bus);

	delete world;
}

void Application::DestroyAllWorlds()
{
	for (World*& world : m_worlds)
	{
		delete world;
	}

	m_worlds.clear();
}

void Application::Tick()
{
	// app loop
    
	for (World*& world : m_worlds)
	{
		if (world->GetInitState())
		{
			world->Tick();
		}
	}

	// UI

	m_window.BeginImgui();

	for (World*& world : m_worlds)
	{
		if (world->GetInitState())
		{
			world->TickUI();
		}
	}

	m_window.EndImgui();

	// Events

	m_queue.Execute();

	// OS
	
	m_audio.Tick();

	m_window.EndFrame();
	m_window.PumpEvents();
}

Window&     Application::GetWindow()         { return m_window; }
AudioWorld& Application::GetAudio()          { return m_audio; }
EventBus&   Application::GetRootEventBus()   { return m_bus; }
EventQueue& Application::GetRootEventQueue() { return m_queue; }
Console&    Application::GetConsole()        { return m_console; }

const std::vector<World*>& Application::GetWorlds() const { return m_worlds; }

World* Application::GetWorld(const char* name)
{
	for (World* world : m_worlds)
	{
		if (   world->GetName() 
			&& strcmp(world->GetName(), name) == 0)
		{
			return world;
		}
	}

	return nullptr;
}
