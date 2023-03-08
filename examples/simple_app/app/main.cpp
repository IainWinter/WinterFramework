#include "global.h"
#include "render.h"

EventQueue _event;
EntityWorld _world;
PhysicsWorld _physics;
AudioWorld _audio;
Window _window;

Camera _camera;

void init()
{

}

void setup()
{

}

void setup_inputs()
{

}

void loop()
{

}

int main()
{
	init();

	global_init(_window, _event, _physics, _world, _audio);
	render_init();

	setup_inputs();
	setup();

	while (is_running())
	{
		tick_pre(_window);

		loop();
		render(_camera, _world);

		tick_frame(_window, _event, _physics, _world, _audio);
	}

	render_dnit();
	global_dnit(_window, _world, _audio);

	return 0;
}