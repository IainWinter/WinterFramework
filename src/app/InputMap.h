#pragma once

#include "Common.h"
#include "Log.h"
#include "util/comparable_str.h"
#include <unordered_map>
#include <array>

// need context

// doesnt handle mapping mouse
// those needs custom behaviours

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

	struct InputAxis
	{
		std::unordered_map<int, vec2> components; // sum of State[code] * component is axis state
		float deadzone = 0.f;
	};

	struct VirtualAxis
	{
		std::vector<InputName> children;
	};

	struct InputContext
	{
		// mapping of name to virtual axis
		// these are groups of other axes so each can have its own processing
		// then be combined
		std::unordered_map<InputName, VirtualAxis> VirtualAxes;

		// mapping of name to axis
		// all buttons can be represented as axies with a deadzone
		// multibutton combos can be thought as an axis with components ('A', .5) and ('B', .5) and a dead zone = 1
		std::unordered_map<InputName, InputAxis> Axes;

		// mapping of code to inputname
		// allows us to skip a search through all components of each axis to find a mapping
		std::unordered_map<int, InputName> Mapping;

		// raw states
		std::array<float, NUMBER_OF_STATES> State;
	};

	void CreateContext();
	void DestroyContext();
	void SetCurrentContext(InputContext* context);
	InputContext* GetContext();

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