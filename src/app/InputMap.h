#pragma once

#include "Common.h"
#include "Log.h"
#include <unordered_map>
#include <array>

// need context

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
		return std::hash<std::string>{}(s.str); // does this make a copy?
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

namespace Input
{
	enum
	{
		NUMBER_OF_STATES = 1 + cAXIS_MAX + SDL_NUM_SCANCODES
	};

	float GetButton(InputName button);
	vec2  GetAxis  (InputName axis);

	void CreateAxis(InputName name);
	void CreateVirtualAxis(InputName name);

	bool AxisExists(InputName axis);
	bool VirtualAxisExists(InputName axis);

	// Set the deadzone of an axis. Not a virtual aixs, use this for the components
	void SetDeadzone(InputName axis, float deadzone);

	// Combine multiple axes into a single virtual axis
	void SetVirtualAxisComponent(InputName axis, InputName component);
	
	// translation helpers

	int GetCode(KeyboardInput scancode);
	int GetCode(ControllerInput input);

	InputName GetMapping(int code);
	InputName GetMapping(KeyboardInput scancode);
	InputName GetMapping(ControllerInput input);

	void SetAxisComponent(InputName axis, int code, vec2 weight = vec2(1.f, 0.f));
	
	// Set a key's weight on an axis
	void SetAxisComponent(InputName axis, KeyboardInput scancode, vec2 weight = vec2(1.f, 0.f));
	
	// Set a controller input's weight on an axis
	void SetAxisComponent(InputName axis, ControllerInput input, vec2 weight = vec2(1.f, 0.f));

	// Internal, for framework to set state
	void SetState(int code, float state);
}