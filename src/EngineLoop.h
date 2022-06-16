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
#include "Leveling.h"
#include "Physics.h"
#include "ext/Time.h" // Time shouldnt be an ext...

// holds the application and has some
// functions for updating its state
// this is where global events would attach...

//#define IW_METRICS_TIMER

enum MetricName
{
	TICK,               // value is DeltaTime
	TICK_FIXED_TIME     // value is FixedTime
};

struct event_RecordMetric
{
	MetricName metric; // name
	float timePoint;   // TotalTime
	float value;       // whatever value for metric, see metric name
};

struct EngineLoop
{
protected:
	Application m_app;
	
	bool m_running = true;
	float m_fixedStepAcc = 0.f;

public:
	void on(event_Shutdown& e) { m_running = false; }

	bool Running()
	{
		return m_running;
	}

	void Init()
	{
		// attach system events
		m_app.Attach<event_Shutdown>(this);

		// default app
		m_app.AddModule<Window>(WindowConfig{ "Untitled", 400, 400 }, m_app.GetRootEventQueue());
		m_app.AddModule<LevelManager>(m_app);
		m_app.AddModule<PhysicsWorld>();

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

#ifdef IW_METRICS_TIMER
		LogMetric(TICK, Time::DeltaTime());
#endif

		m_fixedStepAcc += Time::DeltaTime();
		if (m_fixedStepAcc >= Time::FixedTime())
		{
#ifdef IW_METRICS_TIMER
			LogMetric(TICK_FIXED_TIME, Time::FixedTime());
#endif

			m_fixedStepAcc = 0;
			
			TickLevelFixed();
			TickPhysics();
		}

		TickLevel();
		TickLevelUI();
		
		TickFrame();
		TickEvents();
		TickTasks();
	}

	// Interface

protected:
	virtual void _Init() {};
	virtual void _Dnit() {};

private:
	// Level Updates

	void TickLevel()
	{
		for (SystemBase* system : LevelManager::CurrentLevel()->GetSystems())
		{
			system->Update();
		}
	}

	void TickLevelFixed()
	{
		for (SystemBase* system : LevelManager::CurrentLevel()->GetSystems())
		{
			system->FixedUpdate();
		}
	}

	void TickLevelUI()
	{
		Window& window = m_app.GetModule<Window>();

		window.BeginImgui();
		for (SystemBase* system : LevelManager::CurrentLevel()->GetSystems())
		{
			system->UI();
		}
		window.EndImgui();
	}

	// Module updates

	void TickFrame()
	{
		Window& window = m_app.GetModule<Window>();

		window.EndFrame();
		window.PumpEvents();
	}

	void TickPhysics()
	{
		PhysicsWorld& physics = m_app.GetModule<PhysicsWorld>();
		physics.Step(Time::FixedTime());
	}

	void TickEvents()
	{
		m_app.GetRootEventQueue()->execute();
		LevelManager::CurrentLevel()->GetLevelEventQueue()->execute(); // might be issue while loading/unloading in background...
	}

	void TickTasks()
	{
		m_app.GetTaskPool()->TickCoroutines();
	}

#ifdef IW_METRICS_TIMER
	// debug

	void LogMetric(MetricName name, float value)
	{
		m_app.GetRootEventQueue()->send(event_RecordMetric{ name, Time::TotalTime(), value });
	}

#endif
};

template<typename _engine_loop>
void RunEngineLoop()
{
	_engine_loop loop;
	loop.Init();
	while (loop.Running()) loop.Tick();
	loop.Dnit();
}