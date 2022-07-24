#include "app/System.h"

void _log(const char* fmt, ...)
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
	_log("Created System");

	m_world = world;
	Init();
}

void SystemBase::_Dnit()
{
	_log("Destroied System");
	m_world->GetEventBus().detach(this);
	Dnit();
}

void SystemBase::_Activate()
{
	Activate();
}

void SystemBase::_Deactivate()
{
	Deactivate();
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

WindowRef SystemBase::GetWindow()
{
	return m_world->GetWindow();
}

World* SystemBase::GetWorld()
{
	return m_world;
}

void World::Tick()
{
	m_fixedTimeAcc += Time::DeltaTime();

	if (m_fixedTimeAcc >= Time::FixedTime())
	{
		m_fixedTimeAcc = 0.f; // should subtract the fixed time, but that cuases runaway frame drops

		TickFixed();
	}

	TickSystems();

	TickTasks();
	TickEvents();
	TickEntities();
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

void World::TickTasks()
{
	m_tasks.TickCoroutines();
}

void World::TickEvents()
{
	m_queue.execute();
}

void World::TickEntities()
{
	m_entities.ExecuteDeferdDeletions();
}

World::World(
	Application* app,
	event_manager* root)
	: m_app   (app)
	, m_queue (&m_bus)
{
	root->attach_child(&m_bus);

	m_entities.OnAdd   <Rigidbody2D, &World::on_add_rigidbody, World>(this);
	m_entities.OnRemove<Rigidbody2D, &World::on_remove_rigidbody, World>(this);

	_log("Created world");
}

World::~World()
{
	for (SystemBase*& system : m_systems)
	{
		system->_Deactivate();
		system->_Dnit();
		delete system;
	}

	_log("Destroied world");
}

void World::DetachFromRoot(event_manager* root)
{
	root->detach_child(&m_bus);
}

void World::SetActive(bool active)
{
	m_active = active;
}

bool World::IsActive() const
{
	return m_active;
}

Application*   World::GetApplication()  { return m_app; }
EntityWorld&   World::GetEntityWorld()  { return m_entities; }
PhysicsWorld&  World::GetPhysicsWorld() { return m_physics; }
event_queue&   World::GetEventQueue()   { return m_queue; }
event_manager& World::GetEventBus()     { return m_bus; }
TaskPool&      World::GetTaskPool()     { return m_tasks; }
WindowRef      World::GetWindow()       { return WindowRef(&m_app->GetWindow()); }

void World::AttachSystem(SystemBase* system)
{
	m_systems.push_back(system);
	system->_Activate();
}

void World::DetachSystem(SystemBase* system)
{
	system->_Deactivate();
	m_systems.erase(std::find(m_systems.begin(), m_systems.end(), system));
}

void World::DestroySystem(SystemBase* system)
{
	DetachSystem(system);
	system->_Dnit();
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

Application::Application()
	: m_queue  (&m_bus)
	, m_window (WindowConfig{}, &m_queue)
{}

World* Application::CreateWorld()
{
	World* world = new World(this, &m_bus);
	m_worlds.push_back(world);

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
		if (world->IsActive()) world->Tick();
	}

	m_queue.execute();

	// UI

	m_window.BeginImgui();

	for (World*& world : m_worlds)
	{
		if (world->IsActive()) world->TickUI();
	}

	m_window.EndImgui();

	// OS

	m_window.EndFrame();
	m_window.PumpEvents();
}

Window&        Application::GetWindow()         { return m_window; }
event_manager& Application::GetRootEventBus()   { return m_bus; }
event_queue&   Application::GetRootEventQueue() { return m_queue; }