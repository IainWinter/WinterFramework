#include "app/EngineLoop.h"
#include "ext/serial/serial_common.h"

void EngineLoopBase::on(event_Shutdown& e)
{
	m_running = false;
}

void EngineLoopBase::on(event_Key& e)
{
	if (e.repeat == 0)
	{
		HandleInputMapping(GetInputCode(e.keycode), e.state ? 1.f : 0.f);
	}
}

void EngineLoopBase::on(event_Mouse& e)
{
	switch (e.mousecode)
	{
	case MOUSE_LEFT:
	case MOUSE_MIDDLE:
	case MOUSE_RIGHT:
	case MOUSE_X1:
	case MOUSE_X2:
		HandleInputMapping(GetInputCode(e.mousecode), 
			   e.button_left
			|| e.button_middle
			|| e.button_right
			|| e.button_x1
			|| e.button_x2
		);

		break;

	case MOUSE_VEL_POS:

		vec2 pos = Input::MapToViewport(e.screen_x, e.screen_y);

		HandleInputMapping(GetInputCode(MOUSE_POS_X), pos.x);
		HandleInputMapping(GetInputCode(MOUSE_POS_Y), pos.y);
		HandleInputMapping(GetInputCode(MOUSE_VEL_X), e.vel_x);
		HandleInputMapping(GetInputCode(MOUSE_VEL_Y), e.vel_y);

		break;

	case MOUSE_VEL_WHEEL:
		HandleInputMapping(GetInputCode(MOUSE_VEL_WHEEL_X), e.vel_x);
		HandleInputMapping(GetInputCode(MOUSE_VEL_WHEEL_Y), e.vel_y);

		break;

	default:
		assert(false && "a mouse event without a code was sent");
		break;
	}
}

void EngineLoopBase::on(event_Controller& e)
{
	HandleInputMapping(GetInputCode(e.input), e.value);
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

	// attach system events
	app.GetRootEventBus().Attach<event_Shutdown>(this);
	app.GetRootEventBus().Attach<event_Key>(this);
	app.GetRootEventBus().Attach<event_Mouse>(this);
	app.GetRootEventBus().Attach<event_Controller>(this);
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

void EngineLoopBase::HandleInputMapping(int code, float state)
{
	Input::SetState(code, state);

	InputName input = Input::GetMapping(code);

	if (input.size() > 0)
	{
		event_Input e;
		e.name = input;
		e.axis = Input::GetAxis(input);

		app.GetRootEventQueue().Send(e);
	}
}
