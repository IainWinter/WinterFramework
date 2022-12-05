#include "app/System.h"
#include "InternalSystems.h"

void SystemBase::_Init(r<World> world)
{
	log_world("i~\tInitialized System %s", GetName());

	m_world = world;
	Init();
}

void SystemBase::_Dnit()
{
	Dnit();
	m_world.reset();

	log_world("i~\tDeinitialized System %s", GetName());
}

void SystemBase::_OnAttach()
{
	log_world("i~\tAttached System %s", GetName());

	m_active = true;
	OnAttach();
}

void SystemBase::_OnDetach()
{
	m_active = false;
	GetWorld()->GetEventBus().Detach(this);
	OnDetach();

	log_world("i~\tDetached System %s", GetName());
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
	: m_active (false)
    , m_id     (-1)
{
	log_world("i~\tCreated System");
}

SystemBase::~SystemBase()
{
	log_world("i~\tDestroied System %s", GetName());
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

SystemBase* SystemBase::SetName(const std::string& name)
{
	m_name = name;
	return this;
}

const char* SystemBase::GetName() const
{
	return m_name.c_str();
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
	return m_world->GetPhysicsWorld().QueryRay(pos, end);
}

RayQueryResult SystemBase::QueryRay(vec2 pos, vec2 direction, float distance)
{
	return m_world->GetPhysicsWorld().QueryRay(pos, direction, distance);
}

PointQueryResult SystemBase::QueryPoint(vec2 pos, float radius)
{
	return m_world->GetPhysicsWorld().QueryPoint(pos, radius);
}

r<World> SystemBase::GetWorld()
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
    
    // create a default camera just so there is one for First<Camera>(), in future I want to have a scene with a MainCamera type deal
    // allow another system to take care of this
	// this screws with the saving / loading

    //Entity e = m_entities.Create().SetName("Main Camera");
    //e.Add<Transform2D>();
    //e.Add<Camera>(10, 10, 10); // ortho camera as default
}

r<World> World::Make(Application* app, EventBus* root)
{
	return r<World>(new World(app, root));
}

World::~World()
{
	// wait for tasks before destroying systems
	// to keep bound lambda memory alive
	m_tasks.ShutdownAndWait();

	// should figure out what to do with events, prob execute them

	DestroyAllSystems();

	m_bus.DetachFromParent();

	log_world("i~Destroied World %s", GetName());
}

const std::vector<SystemBase*>& World::GetSystems() const
{
    return m_systems;
}

void World::AttachSystem(SystemBase* system)
{
	if (HasSystem(system))
	{
		log_world("e~Error: System is already in world: %s", system->GetName());
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
		log_world("e~Error: System is not in world: %s", system->GetName());
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
		log_world("e~Error: System is not in world: %s", GetName());
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

void World::DestroyAllSystems()
{
	for (SystemBase* system : m_systems)
	if (system->GetInitState())
		system->_OnDetach();

	for (SystemBase* system : m_systems)
	if (system->GetInitState())
		system->_Dnit();

	for (SystemBase* system : m_systems)
		delete system;

	m_systems.clear();
}

void World::Init()
{
	m_init = true;
	InitUninitializedSystems();
	AttachInactiveSystems();
}

void World::SetDebug(bool debug)
{
	log_world("World %s debug set to %s", GetName(), debug ? "true" : "false");
	m_debug = debug;
}

r<World> World::SetName(const std::string& name)
{
	m_name = name;
	return shared_from_this();
}

const char* World::GetName() const
{
	return m_name.c_str();
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
			system->_Init(shared_from_this());
		}
	}
}

void World::AttachDefaultEntityEvents()
{
	// auto add/remove to physics world

	m_entities.OnAdd   <Rigidbody2D>([this](Entity e) { GetPhysicsWorld().Add(e); });
	m_entities.OnRemove<Rigidbody2D>([this](Entity e) { GetPhysicsWorld().Remove(e); });
    
    m_entities.OnAdd<Transform2D>([this](Entity e)
    {
        if (e.Has<Rigidbody2D>())
            e.Get<Rigidbody2D>().SetTransform(e.Get<Transform2D>());
    });
}

void World::ReallocWorlds()
{
	// free all entt memory that could have been allocated across dll bounds
	entt::registry& reg = m_entities.entt();
	reg = {};

	// clear all box2d memory
	m_physics.ReallocWorld();

	// reattach default events
	AttachDefaultEntityEvents();
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
	RemoveAllWorlds();
}

const std::vector<r<World>>& Application::GetWorlds() const
{
	return m_worlds;
}

r<World> Application::CreateWorld()
{
	r<World> world = World::Make(this, &m_bus);
	AttachWorld(world);

	return world;
}

void Application::AttachWorld(r<World> world)
{
	if (HasWorld(world))
	{
		log_app("w~Tried to attach a world to an application that already contains it");
		return;
	}

	if (!world->GetInitState())
	{
		world->Init();
	}

	m_worlds.push_back(world);
}

void Application::DetachWorld(r<World> world)
{
	if (!HasWorld(world))
	{
		log_app("w~Tried to detach a world from an application that doesn't contain it");
		return;
	}

	m_worlds.erase(std::find(m_worlds.begin(), m_worlds.end(), world));
}

r<World> Application::GetWorld(const std::string& name) const
{
	for (const r<World>& world : m_worlds)
	{
		if (world->GetName() == name)
		{
			return world;
		}
	}

	return nullptr;
}

bool Application::HasWorld(const std::string& name) const
{
	return GetWorld(name) != nullptr;
}

bool Application::HasWorld(const r<World>& world) const
{
	return std::find(m_worlds.begin(), m_worlds.end(), world) != m_worlds.end();
}

void Application::AttachPackage(const WorldPackage& package)
{
	for (auto world : package.worlds)
	{
		AttachWorld(world);
	}
}

void Application::DetachPackage(const WorldPackage& package)
{
	for (auto world : package.worlds)
	{
		DetachWorld(world);
	}
}

void Application::RemoveAllWorlds()
{
	m_worlds.clear();
}

void Application::Tick()
{
	// app loop
    
	for (r<World>& world : m_worlds)
	{
		if (world->GetInitState())
		{
			world->Tick();
		}
	}

	// UI

	m_window.BeginImgui();

	for (r<World>& world : m_worlds)
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
