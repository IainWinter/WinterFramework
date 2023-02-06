#include "global.h"
#include "render.h"

EventQueue _event;
EntityWorld _world;
PhysicsWorld _physics;
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

	global_init(_window, _event, _physics, _world);
	render_init();

	setup();
	setup_inputs();

	while (is_running())
	{
		tick_pre(_window);

		loop();
		render(_camera, _world);

		tick_frame(_window, _event, _world, _physics);
	}

	render_dnit();
	global_dnit(_window, _world);

	return 0;
}
