#include "app/EngineLoop.h"

void EngineLoopBase::on(event_Shutdown& e)
{
	m_running = false;
}

void EngineLoopBase::on(event_Key& e)
{
	if (e.repeat == 0)
	{
		HandleInputMapping(Input::GetCode(e.keycode), e.state ? 1.f : 0.f);
	}
}

void EngineLoopBase::on(event_Controller& e)
{
	HandleInputMapping(Input::GetCode(e.input), e.value);
}

void EngineLoopBase::on(event_ConsoleCommand& e)
{
	app.GetConsole().Execute(e.command);
}

void EngineLoopBase::_Init()
{
    // create time
    Time::CreateContext();
    
	// attach system events
	app.GetRootEventBus().Attach<event_Shutdown>(this);
	app.GetRootEventBus().Attach<event_Key>(this);
	app.GetRootEventBus().Attach<event_Controller>(this);
	app.GetRootEventBus().Attach<event_ConsoleCommand>(this);

	// open window and create graphics context, allows sending data to device
	app.GetWindow().Init();

	// init the audio engine
	app.GetAudio().Init();

	// set imgui config flags
	PreInitUI();

	// init Imgui
	app.GetWindow().InitUI();

	// init user UI, load fonts ect.
	InitUI();

	// init user code
	Init();

	// should prob send events from init functions before first tick
}

void EngineLoopBase::_Dnit()
{
    // dnit user worlds
	app.DestroyAllWorlds();
	app.GetRootEventBus().Detach(this);
    
    // dnit user code
	Dnit();
    
    // destroy time
    Time::DestroyContext();
}

bool EngineLoopBase::Tick()
{
	Time::UpdateTime();
	app.Tick();

	return m_running;
}

void EngineLoopBase::HandleInputMapping(int code, float state)
{
	Input::SetState(code, state);

	InputName input = Input::GetMapping(code);

	if (input)
	{
		event_Input e;
		e.name = input;
		e.axis = Input::GetAxis(input);

		app.GetRootEventQueue().Send(e);
	}
}
