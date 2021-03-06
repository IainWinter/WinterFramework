#pragma once

// This file replaces Entry with a much more defined updating pattern using the Leveling structures

// The default interface includes a Window, Physics and Audio (coming)
// In the future this should really be an abstract class, and there would
// be another file with a DefaultEngineLoop that has the Window Physics Audio added
// the interface version should be empty, but this works for now

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

struct EngineLoopBase
{
protected:
	Application app;
	
private:
	bool m_running = true;
	float m_fixedStepAcc = 0.f;

public:
	void on(event_Shutdown& e);
	void on(event_Key& e);
	void on(event_ConsoleCommand& e);

	void _Init();
	void _Dnit();
	bool Tick();

// Interface

protected:
	virtual void Init() {};
	virtual void PreInitUI() {};
	virtual void InitUI() {}; // for loading fonts n such after flags have been set
	virtual void Dnit() {};
};

template<typename _me>
struct EngineLoop : EngineLoopBase
{
	template<typename _e>
	void Attach() { app.GetRootEventBus().Attach<_e, _me>((_me*)this); }

	template<typename _e>
	void Detach() { app.GetRootEventBus().Detach<_e>(this); }
};

template<typename _t>
void RunEngineLoop()
{
	_t loop = _t();

	loop._Init();
	while (loop.Tick()) {}
	loop._Dnit();
}