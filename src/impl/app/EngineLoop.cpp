#include "app/EngineLoop.h"

void EngineLoopBase::on(event_Shutdown& e)
{
	m_running = false;
}

void EngineLoopBase::on(event_Key& e)
{
	if (e.repeat == 0)
	{
		InputName input = Input::Map(e.keycode);

		if (input != InputName::_NONE)
		{
			app.GetRootEventQueue().Send(event_Input{input, e.state ? 1.f : -1.f});
			Input::SetState(input, e.state);
		}
	}
}

void EngineLoopBase::on(event_ConsoleCommand& e)
{
	app.GetConsole().Execute(e.command);
}

void EngineLoopBase::_Init()
{
	// attach system events
	app.GetRootEventBus().Attach<event_Shutdown>(this);
	app.GetRootEventBus().Attach<event_Key>(this);
	app.GetRootEventBus().Attach<event_ConsoleCommand>(this);

	// open window and create graphics context, allows sending data to device
	app.GetWindow().Init();

	// init the audio engine
	app.GetAudio().Init();

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

void EngineLoopBase::_Dnit()
{
	app.GetRootEventBus().Detach(this);
	Dnit();
}

bool EngineLoopBase::Tick()
{
	Time::UpdateTime();
	app.Tick();

	return m_running;
}