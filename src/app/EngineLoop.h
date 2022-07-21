#pragma once

// This file replaces Entry with a much more defined updating pattern using the Leveling structures

// The default interface includes a Window, Physics and Audio (coming)
// In the future this should really be an abstract class, and there would
// be another file with a DefaultEngineLoop that has the Window Physics Audio added
// the interface version should be empty, but this works for now

#define IW_NOLOOP
#include "Entry.h"
#undef IW_NOLOOP

#include "Windowing.h"
#include "Physics.h"

#include "app/System.h"
#include "app/FontMap.h"
#include "app/InputMap.h"

#include "ext/Time.h" // Time shouldnt be an ext...
#include "util/metrics.h" // this should only be in iw_debugger mode

// holds the application and has some
// functions for updating its state
// this is where global events attach...

struct EngineLoop
{
protected:
	Application m_app;
	
	bool m_running = true;
	float m_fixedStepAcc = 0.f;

public:
	void on(event_Shutdown& e)
	{
		m_running = false;
	}

	void on(event_Key& e)
	{
		if (e.repeat == 0)
		{
			// this may not want to send on root queue?

			InputName input = m_app.GetModule<InputMap>().Map(e.keycode);
			m_app.Send(event_Input{ input, e.state ? 1.f : -1.f });
		}
	}

	bool Running()
	{
		return m_running;
	}

	void Init()
	{
		// attach system events
		m_app.Attach<event_Shutdown>(this);
		m_app.Attach<event_Key>(this);

		// default app
		m_app.AddModule<Window>(WindowConfig{ "Untitled", 400, 400 }, m_app.GetRootEventQueue());
		m_app.AddModule<LevelManager>(m_app);
		m_app.AddModule<PhysicsWorld>();
		m_app.AddModule<FontMap>();
		m_app.AddModule<InputMap>();

		// default level
		m_app.GetModule<LevelManager>().CreateLevel();

		// open window and create graphics context, allows sending data to device
		m_app.GetModule<Window>().Init();

		// init user code
		_Init();

		// init the default level
		m_app.GetModule<LevelManager>().InitLevel(LevelManager::CurrentLevel());

		// init UI, defered for user set Imgui config flags
		m_app.GetModule<Window>().InitUI();

		_InitUI();

		// send events from init functions before first tick
		TickEvents();
	}

	void Dnit()
	{
		// doesnt dnit window, should this delete all modules?
		m_app.GetModule<LevelManager>().Destroy(); // delete all entities before window (glcontext) is destroied
		m_app.Detach(this);
		_Dnit();
	}

	void Tick()
	{
		Time::UpdateTime();

		float physicsTicks = 0;

		m_fixedStepAcc += Time::DeltaTime();
		if (m_fixedStepAcc >= Time::FixedTime())
		{
			physicsTicks += 1;
			m_fixedStepAcc = 0.f; // so this is incorrect if the physics should run twice a frame, but if the physics is whats causing a slow frame, it causes a feedback loop...
			//m_fixedStepAcc -= Time::FixedTime();

			TickLevelFixed();
			TickPhysics();
		}

		TickLevel();
		TickLevelUI();
		
		TickTasks();
		TickEvents();
		TickDefered();
		
		TickFrame();

		TickLastTransforms();
	}

	// Interface

protected:
	virtual void _Init() {};
	virtual void _InitUI() {}; // for loading fonts n such after flags have been set
	virtual void _Dnit() {};

private:
	// Level Updates

#define TIME_SCOPE(name) scope_timer temp_scope_time_a = m_app.TimeScope(name)
#define TIME_SCOPE2(name) scope_timer temp_scope_time_b = m_app.TimeScope(name)
#define TIME_SCOPE3(name) scope_timer temp_scope_time_c = m_app.TimeScope(name)

	void TickLevel()
	{
		TIME_SCOPE("Tick Level");

		for (SystemBase* system : LevelManager::CurrentLevel()->GetSystems())
		{
			TIME_SCOPE2(system->GetName());
			system->Update();
		}
	}

	void TickLevelFixed()
	{
		TIME_SCOPE("Tick Level Fixed");

		for (SystemBase* system : LevelManager::CurrentLevel()->GetSystems())
		{
			TIME_SCOPE2(system->GetName());
			system->FixedUpdate();
		}
	}

	void TickLevelUI()
	{
		TIME_SCOPE("Tick Level UI");

		Window& window = m_app.GetModule<Window>();

		window.BeginImgui();
		for (SystemBase* system : LevelManager::CurrentLevel()->GetSystems())
		{
			TIME_SCOPE2(system->GetName());
			system->UI();
			system->Debug();
		}
		window.EndImgui();
	}

	// Module updates

	void TickFrame()
	{
		TIME_SCOPE("Tick Window");

		Window& window = m_app.GetModule<Window>();

		window.EndFrame();
		window.PumpEvents();
	}

	void TickPhysics()
	{
		TIME_SCOPE("Tick Physics");

		PhysicsWorld& physics = m_app.GetModule<PhysicsWorld>();
		physics.Step(Time::FixedTime());
	}

	void TickEvents()
	{
		TIME_SCOPE("Tick Events");

		m_app.GetRootEventQueue()->execute();
		LevelManager::CurrentLevel()->GetLevelEventQueue()->execute(); // might be issue while loading/unloading in background...
	}

	void TickTasks()
	{
		TIME_SCOPE("Tick Tasks");
		m_app.GetTaskPool()->TickCoroutines();
	}

	void TickDefered()
	{
		TIME_SCOPE("Tick Defered");
		// technically the entityworld at the root App level should also get ticked, but I dont think it will ever be needed
		LevelManager::CurrentLevel()->GetWorld()->ExecuteDeferdDeletions();
	}

	void TickLastTransforms()
	{
		TIME_SCOPE("Tick Last Transform Update");

		for (auto [transform] : LevelManager::CurrentLevel()->GetWorld()->Query<Transform2D>())
		{
			transform.UpdateLastFrameData();
		}
	}
};

template<typename _engine_loop>
void RunEngineLoop()
{
	_engine_loop loop;
	loop.Init();
	while (loop.Running()) loop.Tick();
	loop.Dnit();
}