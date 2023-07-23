#pragma once

#include "app/Application.h"
#include "ext/InputEventHandler.h"

// Holds the Application and has some functions for updating its state.
// This is where global events attach

class EngineLoopBase
{  
public:
	void on(event_Shutdown& e);
	void on(event_ConsoleCommand& e);

	void Init();
	void Dnit();
	bool Tick();

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
class EngineLoop : public EngineLoopBase
{
protected:
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
