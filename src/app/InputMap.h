#pragma once

#include "Common.h"
#include <unordered_map>
#include <array>

// doesnt handle mapping mouse
// those needs custom behaviours

enum class InputName
{
	_NONE,

	UP,
	DOWN,
	RIGHT,
	LEFT,

	AIM_X,
	AIM_Y,

	AIM,
	MOVE,

	ATTACK,
	ATTACK_ALT,

	ESCAPE
};

struct event_Input
{
	InputName name;
	union
	{
		float state; // for 1d inputs (buttons)
		vec2 axis;   // for 2d inputs (axies)
	};

	bool enabled() { return state == 1.f; }
};

// doesnt handle mouse right now

namespace Input
{
	enum
	{
		NUMBER_OF_STATES = 1 + cAXIS_MAX + SDL_NUM_SCANCODES
	};

	float GetButton(InputName button);
	vec2  GetAxis  (InputName axis);

	void SetDeadzone(InputName axis, float deadzone);

	InputName _GetMapping(int code);
	void      _SetMapping(InputName button, int code);
	void      _SetAxisComponent(InputName axis, int code, vec2 component);
	
	void      _SetState(int code, float state);

	// translation helpers

	int GetCode(KeyboardInput scancode);
	int GetCode(ControllerInput button);

	InputName GetMapping(KeyboardInput scancode);
	void      SetMapping(InputName name, KeyboardInput scancode);
	void      SetAxisComponent(InputName axis, KeyboardInput scancode, vec2 component);
	
	InputName GetMapping(ControllerInput scancode);
	void      SetMapping(InputName name, ControllerInput button);
	void      SetAxisComponent(InputName axis, ControllerInput button, vec2 component);
}