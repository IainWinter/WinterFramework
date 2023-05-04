#include "app/EngineLoop.h"
#include "ext/serial/serial_common.h"

#include "Rendering.h"

void EngineLoopBase::on(event_Shutdown& e)
{
	m_running = false;
}

void EngineLoopBase::on(event_ConsoleCommand& e)
{
	app.console.Execute(e.command);
}

void EngineLoopBase::Init()
{
    // create framework contexts
	meta::CreateContext();
	meta::register_meta_types();
	register_common_types();

    Time::CreateContext();
	Render::CreateContext();
	Asset::CreateContext();
	File::CreateContext();

	delete m_inputHandler;
    m_inputHandler = new InputEventHandler(&app.input, &app.event);
    
	// attach system events
	app.bus.Attach<event_Shutdown>(this);
	app.bus.Attach<event_ConsoleCommand>(this);

	// open window and create graphics context
	app.window.Init();

	// init the audio engine
	app.audio.Init();

	// set imgui flags
	_PreInitUI();

	// init Imgui
	app.window.InitUI();

	// init user UI, load fonts
	_InitUI();

	// init user code
	_Init();
}

void EngineLoopBase::Dnit()
{
    // dnit user worlds
	app.bus.Detach(this);

    // dnit user code
	_Dnit();
    
    // destroy framework contexts
    Time::DestroyContext();
	Asset::DestroyContext();
	File::DestroyContext();
	Render::DestroyContext();

	delete m_inputHandler;
}

bool EngineLoopBase::Tick()
{
	Time::UpdateTime();
	Render::SetWindowSize(app.window.Width(), app.window.Height());

	app.Tick();

	return m_running;
}
