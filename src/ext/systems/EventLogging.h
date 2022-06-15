#pragma once

#include "Leveling.h"
#include "Windowing.h"

struct EventLogging : System<EventLogging>
{
	void Init()
	{
		Attach<event_Shutdown>();
		Attach<event_WindowResize>();
		Attach<event_Mouse>();
		Attach<event_Input>();
	}

	void on(event_Shutdown& e)
	{
		printf(
			"[Event] Recieved: event_Shutdown {}\n");
	}

	void on(event_WindowResize& e)
	{
		printf(
			"[Event] Recieved: event_WindowResize {"
			"\n\twidth %d"
			"\n\theight %d"
			"\n}\n", e.width, e.height);
	}

	void on(event_Mouse& e)
	{
		printf(
			"[Event] Recieved: event_Mouse {"
			"\n\tpixel_x %d"
			"\n\tpixel_y %d"
			"\n\tscreen_x %f"
			"\n\tscreen_y %f"
			"\n\tvel_x %f"
			"\n\tvel_y %f"
			"\n\tbutton_left %d"
			"\n\tbutton_middle %d"
			"\n\tbutton_right %d"
			"\n\tbutton_x1 %d"
			"\n\tbutton_x2 %d"
			"\n\tbutton_repeat %d"
			"\n}\n", e.pixel_x, e.pixel_y, e.screen_x, e.screen_y,
			e.vel_x, e.vel_y, e.button_left, e.button_middle,
			e.button_right, e.button_x1, e.button_x2, e.button_repeat);
	}

	void on(event_Input& e)
	{
		static std::unordered_map<InputName, const char*> names =
		{
			{ InputName::_NONE, "None" },
			{ InputName::UP,    "Up" },
			{ InputName::DOWN,  "Down" },
			{ InputName::RIGHT, "Right" },
			{ InputName::LEFT,  "Left" },
		};

		printf(
			"[Event] Recieved: event_Input {"
			"\n\tname %s"
			"\n\tstate %f"
			"\n}\n", names.at(e.name), e.state);
	}
};