#include "global.h"

#include "app/InputEventHandler.h"
#include "app/Time.h"
#include "app/InputMap.h"
#include "util/filesystem.h"
#include "ext/AssetStore.h"
#include "ext/rendering/SimpleRender.h"

bool _running = true;
InputEventHandler* _input;

void context_init()
{
	wlog::CreateContext();
	meta::CreateContext();

	Time::CreateContext();
	Input::CreateContext();
	Render::CreateContext();
	
	File::CreateContext();
	File::SetAssetPath(ASSET_ROOT_PATH);
	
	Asset::CreateContext();
    
    InitSimpleRendering();
}

void context_dnit()
{
    DnitSimpleRendering();
        
	Asset::DestroyContext();
	File::DestroyContext();

	Time::DestroyContext();
	Input::DestroyContext();
	Render::DestroyContext();

	meta::DestroyContext();
	wlog::DestroyContext();
}

void _shutdown(event_Shutdown& e)
{
	_running = false;
}

void attach_callbacks(EventBus& bus)
{
	bus.Attach<event_Shutdown>(_shutdown);
}

void global_init(Window& window, EventQueue& queue, PhysicsWorld& physics, EntityWorld& world, AudioWorld& audio)
{
	context_init();
	attach_callbacks(*queue.GetBus());

    _input = new InputEventHandler(&queue);
    
	window.SetEventQueue(&queue);
	window.Init();
	window.InitUI();

	world.OnAdd   <Rigidbody2D>([&physics](Entity entity) { physics.Add(entity); });
	world.OnRemove<Rigidbody2D>([&physics](Entity entity) { physics.Remove(entity); });

	audio.Init();
}

void global_dnit(Window& window, EntityWorld& world, AudioWorld& audio)
{
	world.Clear();
	audio.Dnit();

	context_dnit();

	// This releases opengl context, so do last
	window.Dnit();
}

void tick_pre(Window& window)
{
	Time::UpdateTime();
	Render::SetWindowSize(window.Width(), window.Height());

	window.BeginImgui();
}

void tick_frame(Window& window, EventQueue& event, PhysicsWorld& physics, EntityWorld& world, AudioWorld& audio)
{
	physics.Tick(Time::DeltaTime());
	audio.Tick();

	window.PumpEvents();
	window.EndImgui();
	window.EndFrame();

	event.Execute();

	world.ExecuteDeferdDeletions();
}

bool is_running()
{
	return _running;
}