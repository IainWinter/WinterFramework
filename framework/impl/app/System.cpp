#include "app/System.h"
#include "InternalSystems.h"

void world_log(const char* fmt, ...)
{
	printf("[World] ");

	va_list args;
	va_start(args, fmt);
	vprintf(fmt, args);
	va_end(args);

	printf("\n");
}

void SystemBase::_Init(World* world)
{
	world_log("\tInitialized System");

	m_world = world;
	Init();
}

void SystemBase::_Dnit()
{
	Dnit();
	m_world = nullptr;

	world_log("\tDeinitialized System");
}

void SystemBase::_OnAttach()
{
	world_log("\tAttached System");

	m_active = true;
	OnAttach();
}

void SystemBase::_OnDetach()
{
	m_active = false;
	m_world->GetEventBus().Detach(this);
	OnDetach();

	world_log("\tDetached System");
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

SystemBase::SystemBase()
	: m_world  (nullptr)
	, m_active (false)
{
	world_log("\tCreated System");
}

SystemBase::~SystemBase()
{
	world_log("\tDestroied System");
}

bool SystemBase::GetInitState() const
{
	return !!m_world;
}

bool SystemBase::GetActiveState() const
{
	return m_active;
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

int SystemBase::PlaySound(const std::string& eventPath)
{
	return GetAudio().Play(eventPath);
}

void SystemBase::StopSound(int soundHandle)
{
	GetAudio().Free(soundHandle);
}

void SystemBase::PauseSound(int soundHandle)
{
	GetAudio().Stop(soundHandle);
}

WindowRef SystemBase::GetWindow()
{
	return m_world->GetWindow();
}

Audio& SystemBase::GetAudio()
{
	return m_world->GetAudio();
}

World* SystemBase::GetWorld()
{
	return m_world;
}

World::World(
	Application* app, EventBus* root
)
	: m_app          (app)
	, m_queue        (&m_bus)
	, m_fixedTimeAcc (0.f)
	, m_init         (false)
{
	world_log("Created World");

	// allow app events to propagate down to world

	root->ChildAttach(&m_bus);

	// auto add/remove bodies to physics

	m_entities.OnAdd   <Rigidbody2D, &World::on_add_rigidbody, World>(this);
	m_entities.OnRemove<Rigidbody2D, &World::on_remove_rigidbody, World>(this);

	// auto set audio on component

	//m_entities.OnAdd<AudioEmitter, &World::on_add_audio_emitter, World>(this);

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

	for (SystemBase* system : m_systems)
	if (system->GetInitState())
		system->_OnDetach();

	for (SystemBase* system : m_systems)
	if (system->GetInitState())
		system->_Dnit();

	for (SystemBase* system : m_systems)
		delete system;

	world_log("Destroied World");
}

void World::DetachFromRoot(EventBus* root)
{
	root->ChildDetach(&m_bus);
}

void World::AttachSystem(SystemBase* system)
{
	if (ContainsSystem(system))
	{
		world_log("Error: System is already in world");
		return;
	}

	m_systems.push_back(system);
	
	if (system->GetInitState())
	{
		system->_OnAttach();
	}
}

void World::DetachSystem(SystemBase* system)
{
	if (!ContainsSystem(system))
	{
		world_log("Error: System is not in world");
		return;
	}

	if (system->GetInitState())
	{
		system->_OnDetach();
	}

	m_systems.erase(std::find(m_systems.begin(), m_systems.end(), system));
}

void World::DestroySystem(SystemBase* system)
{
	if (!ContainsSystem(system))
	{
		world_log("Error: System is not in world");
		return;
	}

	DetachSystem(system);
	
	if (system->GetInitState())
	{
		system->_Dnit();
	}

	delete system;
}

bool World::ContainsSystem(SystemBase* system) const
{
	return std::find(m_systems.begin(), m_systems.end(), system) != m_systems.end();
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

Application*  World::GetApplication()     { return m_app; }
WindowRef     World::GetWindow()          { return WindowRef(&m_app->GetWindow()); }
Audio&        World::GetAudio()           { return m_app->GetAudio(); }

EntityWorld&  World::GetEntityWorld()     { return m_entities; }
PhysicsWorld& World::GetPhysicsWorld()    { return m_physics; }
TaskPool&     World::GetTaskPool()        { return m_tasks; }

EventBus&     World::GetEventBus()        { return m_bus; }
EventQueue&   World::GetEventQueue()      { return m_queue; }

bool          World::GetInitState() const { return m_init; }

void World::Tick()
{
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
}

void World::TickUI()
{
	TickSystemsUI();
	TickSystemsDebug();
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

void World::on_add_rigidbody(entt::registry& reg, entt::entity e)
{
	Entity entity = m_entities.Wrap((u32)e);
	m_physics.Add(entity);
}

void World::on_remove_rigidbody(entt::registry& reg, entt::entity e)
{
	Entity entity = m_entities.Wrap((u32)e);
	m_physics.Remove(entity);
}

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
	for (World*& world : m_worlds)
	{
		delete world;
	}
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

	m_queue.Execute();

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

	// OS
	
	m_audio.Tick();

	m_window.EndFrame();
	m_window.PumpEvents();
}

Window&     Application::GetWindow()         { return m_window; }
Audio&      Application::GetAudio()          { return m_audio; }
EventBus&   Application::GetRootEventBus()   { return m_bus; }
EventQueue& Application::GetRootEventQueue() { return m_queue; }
Console&    Application::GetConsole()        { return m_console; }