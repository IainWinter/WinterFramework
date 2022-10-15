#pragma once

#include "Common.h"
#include <unordered_map>
#include <array>

// doesnt handle mapping mouse
// those needs custom behaviours

// this is a helper class for easy if statements between these const char*s
// most of the time they should be in the string table, so this is just an int comparison
struct comparable_const_char
{
	const char* str;

	comparable_const_char() : str(nullptr) {}
	comparable_const_char(const char* str) : str(str) {}
	operator const char* () const { return str; }

	bool operator==(const char* s) { return str == s || strcmp(str, s) == 0; }
	bool operator!=(const char* s) { return !operator==(s); }
};

template<>
struct std::hash<comparable_const_char>
{
	std::size_t operator()(const comparable_const_char& s) const noexcept
	{
		return std::hash<std::string>{}(s.str);
	}
};

using InputName = comparable_const_char;

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