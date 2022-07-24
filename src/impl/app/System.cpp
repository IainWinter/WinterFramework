#include "app/System.h"

void SystemBase::_Init(r<World> world)
{
	m_world = world;
	Init();
}

void SystemBase::_Dnit()
{
	m_world->GetEventBus().detach(this);
	Dnit();
}

void SystemBase::_Attach()
{
	Attach();
}

void SystemBase::_Detach()
{
	Detach();
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

SystemBase::~SystemBase()
{
	if (m_world)
	{
		_Detach();
		_Dnit();
	}
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

r<World> SystemBase::GetWorld()
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
	for (r<SystemBase>& system : m_systems)
	{
		system->_FixedUpdate();
	}

	m_physics.Tick(Time::FixedTime());
}

void World::TickSystems()
{
	for (r<SystemBase>& system : m_systems)
	{
		system->_Update();
	}
}

void World::TickSystemsUI()
{
	for (r<SystemBase>& system : m_systems)
	{
		system->_UI();
	}
}

void World::TickSystemsDebug()
{
	for (r<SystemBase>& system : m_systems)
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
	r<Application> app,
	event_manager* root)
	: m_app   (app)
	, m_queue (&m_bus)
{
	root->attach_child(&m_bus);
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

r<Application> World::GetApplication()  { return m_app; }
EntityWorld&   World::GetEntityWorld()  { return m_entities; }
PhysicsWorld&  World::GetPhysicsWorld() { return m_physics; }
event_queue&   World::GetEventQueue()   { return m_queue; }
event_manager& World::GetEventBus()     { return m_bus; }
TaskPool&      World::GetTaskPool()     { return m_tasks; }
WindowRef      World::GetWindow()       { return WindowRef(&m_app->GetWindow()); }

void World::AttachSystem(r<SystemBase> system)
{
	m_systems.push_back(system);
	system->_Attach();
}

void World::DetachSystem(r<SystemBase> system)
{
	system->_Detach();
	m_systems.erase(std::find(m_systems.begin(), m_systems.end(), system));
}

void World::DestroySystem(r<SystemBase> system)
{
	DetachSystem(system);
	system->_Dnit();
}

Application::Application()
	: m_queue(&m_bus)
{}

r<World> Application::CreateWorld()
{
	r<World> world = mkr<World>(shared_from_this(), &m_bus);
	m_worlds.push_back(world);

	return world;
}

void Application::DestroyWorld(r<World> world)
{
	m_worlds.erase(std::find(m_worlds.begin(), m_worlds.end(), world));
	world->DetachFromRoot(&m_bus);
}

void Application::Tick()
{
	for (r<World>& world : m_worlds)
	{
		if (world->IsActive()) world->Tick();
	}

	m_window.BeginImgui();

	for (r<World>& world : m_worlds)
	{
		if (world->IsActive()) world->TickUI();
	}

	m_window.EndImgui();

	m_queue.execute();

	m_window.EndFrame();
	m_window.PumpEvents();
}

Window&        Application::GetWindow()         { return m_window; }
event_manager& Application::GetRootEventBus()   { return m_bus; }
event_queue&   Application::GetRootEventQueue() { return m_queue; }