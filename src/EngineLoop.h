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

		// init user code
		_Init();

		// init the default level
		m_app.GetModule<LevelManager>().InitLevel(LevelManager::CurrentLevel());

		// open window, defered for Imgui config flags before calling init
		m_app.GetModule<Window>().Init();
	}

	void Dnit()
	{
		// doesnt dnit window, should this delete all modules?

		m_app.Detach(this);
		_Dnit();
	}

	void Tick()
	{
		Time::UpdateTime();

		m_fixedStepAcc += Time::DeltaTime();
		if (m_fixedStepAcc >= Time::FixedTime())
		{
			m_fixedStepAcc = 0;
			
			TickLevelFixed();
			TickPhysics();
		}

		TickLevel();
		TickLevelUI();
		
		TickFrame();

		m_app.GetRootEventQueue()->execute();
		LevelManager::CurrentLevel()->GetLevelEventQueue()->execute(); // might be issue while loading/unloading in background...
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
};

template<typename _engine_loop>
void RunEngineLoop()
{
	_engine_loop loop;
	loop.Init();
	while (loop.Running()) loop.Tick();
	loop.Dnit();
}