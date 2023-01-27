#include "app/EngineLoop.h"
#include "ext/serial/serial_common.h"

void EngineLoopBase::on(event_Shutdown& e)
{
	m_running = false;
}

void EngineLoopBase::on(event_ConsoleCommand& e)
{
	app.GetConsole().Execute(e.command);
}

void EngineLoopBase::on(event_CreateEntity& e)
{
	Entity entity = e.world->GetEntityWorld().Create();
	if (e.callback) e.callback(entity);
}

void EngineLoopBase::_Init()
{
    // create framework contexts
	meta::CreateContext();
	meta::register_meta_types();
	register_common_types();

    Time::CreateContext();
	Render::CreateContext();
	Asset::CreateContext();
	File::CreateContext();
	Input::CreateContext();
	wlog::CreateContext();

	// init debug renders
	Debug::Init();

    m_inputHandler = InputEventHandler(&app.GetRootEventQueue());
    
	// attach system events
	app.GetRootEventBus().Attach<event_Shutdown>(this);
	app.GetRootEventBus().Attach<event_ConsoleCommand>(this);
	app.GetRootEventBus().Attach<event_CreateEntity>(this);

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
	app.RemoveAllWorlds();
	app.GetRootEventBus().Detach(this);
    
    // dnit user code
	Dnit();
    
	// dnit debug renders
	Debug::Dnit();

    // destroy framework contexts
    Time::DestroyContext();
	Asset::DestroyContext();
	File::DestroyContext();
	Input::DestroyContext();
	Render::DestroyContext();
	wlog::DestroyContext();
}

bool EngineLoopBase::Tick()
{
	Time::UpdateTime();

	// update render state
	// put this in a better spot
	Render::GetContext()->window_width  = app.GetWindow().Width();
	Render::GetContext()->window_height = app.GetWindow().Height();

	app.Tick();

	return m_running;
}
