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
//#include "util/metrics.h" // this should only be in iw_debugger mode

// holds the application and has some
// functions for updating its state
// this is where global events attach...

struct EngineLoop
{
protected:
	Application app;
	
private:
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
			InputName input = Input::Map(e.keycode);
			app.Send(event_Input{ input, e.state ? 1.f : -1.f });
		}
	}

	bool Running()
	{
		return m_running;
	}

	void _Init()
	{
		// attach system events
		app.Attach<event_Shutdown>(this);
		app.Attach<event_Key>(this);

		// open window and create graphics context, allows sending data to device
		app.GetWindow().Init();

		// init user code
		Init();

		// set imgui config flags
		PreInitUI();

		// init Imgui
		app.GetWindow().InitUI();

		// init user UI, load fonts ect.
		InitUI();

		// should prob send events from init functions before first tick
	}

	void _Dnit()
	{
		app.Detach(this);
		Dnit();
	}

	void Tick()
	{
		Time::UpdateTime();
		app.Tick();
	}

// Interface

protected:
	virtual void Init() {};
	virtual void PreInitUI() {};
	virtual void InitUI() {}; // for loading fonts n such after flags have been set
	virtual void Dnit() {};
};

template<typename _engine_loop>
void RunEngineLoop()
{
	_engine_loop loop;
	loop._Init();
	while (loop.Running()) loop.Tick();
	loop._Dnit();
}