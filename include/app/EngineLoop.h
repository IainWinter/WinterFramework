#pragma once

// This file replaces Entry with a much more defined updating pattern using the Leveling structures

// The default interface includes a Window, Physics and Audio
// In the future this should really be an abstract class, and there would
// be another file with a DefaultEngineLoop that has the Window Physics Audio added
// the interface version should be empty, but this works for now

#include "Windowing.h"
#include "Physics.h"
#include "Input.h"
#include "Clock.h"

#include "app/Application.h"
#include "app/FontMap.h"

#include "ext/InputEventHandler.h"
#include "ext/AssetStore.h"

// holds the application and has some
// functions for updating its state
// this is where global events attach...

struct EngineLoopBase
{  
public:
	void on(event_Shutdown& e);
	void on(event_ConsoleCommand& e);

	void Init();
	void Dnit();
	bool Tick();

// Interface

protected:
	virtual void _Init() {};
	virtual void _PreInitUI() {};
	virtual void _InitUI() {}; // for loading fonts n such after flags have been set
	virtual void _Dnit() {};

protected:
	Application app;

private:
	bool m_running = true;
	InputEventHandler* m_inputHandler = nullptr;
};

template<typename _me>
struct EngineLoop : EngineLoopBase
{
	template<typename _e>
	void Attach() { app.bus.template Attach<_e, _me>((_me*)this); }

	template<typename _e>
	void Detach() { app.bus.template Detach<_e>(this); }
};

template<typename _t>
void RunEngineLoop()
{
	_t loop = _t();

	loop.Init();
	while (loop.Tick()) {}
	loop.Dnit();
}
