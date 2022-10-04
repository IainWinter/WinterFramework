#pragma once

#include "app/System.h"
#include "Windowing.h"
#include "Log.h"

struct EventLogging : System<EventLogging>
{
	void Init()
	{
		Attach<event_Shutdown>();
		Attach<event_WindowResize>();
		Attach<event_Mouse>();
		//Attach<event_Input>();
	}

	void on(event_Shutdown& e)
	{
		log_event(
			"Recieved: event_Shutdown {}");
	}

	void on(event_WindowResize& e)
	{
		log_event(
			"Recieved: event_WindowResize {"
			"\n\twidth %d"
			"\n\theight %d"
			"\n}", e.width, e.height);
	}

	void on(event_Mouse& e)
	{
		log_event(
			"Recieved: event_Mouse {"
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
			"\n}", e.pixel_x, e.pixel_y, e.screen_x, e.screen_y,
			e.vel_x, e.vel_y, e.button_left, e.button_middle,
			e.button_right, e.button_x1, e.button_x2, e.button_repeat);
	}

	//void on(event_Input& e)
	//{
	//	static std::unordered_map<InputName, const char*> names =
	//	{
	//		{ InputName::_NONE, "None" },
	//		{ InputName::UP,    "Up" },
	//		{ InputName::DOWN,  "Down" },
	//		{ InputName::RIGHT, "Right" },
	//		{ InputName::LEFT,  "Left" },
	//	};

	//	log_event(
	//		"[Event] Recieved: event_Input {"
	//		"\n\tname %s"
	//		"\n\tstate %f"
	//		"\n}\n", names.at(e.name), e.state);
	//}
};