#pragma once

#include "Common.h"
#include "util/context.h"
#include <unordered_map>
#include <array>

//
// some oddities
//		Mapping can be overwritten if the two Group axes use the same buttons
//		
//		Should rename 'Virtual Axis' to just Axis, and 'Input Axis' to 'Input Group' or something along those lines


using InputName = std::string;

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
	struct InputCode
	{
		int code;

		operator int() const {
			return code;
		}

		InputCode()                      : code(-1)                  {}
		InputCode(int code)              : code(code)                {}
		InputCode(KeyboardInput input)   : code(GetInputCode(input)) {}
		InputCode(MouseInput input)      : code(GetInputCode(input)) {}
		InputCode(ControllerInput input) : code(GetInputCode(input)) {}
	};
}

namespace std
{
	template <>
	struct hash<Input::InputCode>
	{
		size_t operator()(const Input::InputCode& code) const
		{
			return (int)code;
		}
	};
}

namespace Input
{
	struct InputAxisSettings
	{
		// round to 0 if below this length
		float deadzone = 0.f;

		// limit to a unit vector
		bool limitToUnit = true;

		// normalize to a unit vector
		bool normalized = false;

		// if this is empty, it will be accepted by any active mask
		std::string mask;

		bool operator==(const InputAxisSettings& other) const;
		bool operator!=(const InputAxisSettings& other) const;
	};

	struct InputAxis
	{
		std::unordered_map<int, vec2> components; // (code, component) -> sum of State[code] * component is axis state
		InputAxisSettings settings;

		bool operator==(const InputAxis& other) const;
		bool operator!=(const InputAxis& other) const;
	};

	struct AxisGroup
	{
		std::vector<InputName> axes;
		InputAxisSettings settings;

		bool operator==(const AxisGroup& other) const;
		bool operator!=(const AxisGroup& other) const;
	};

	struct InputContext : wContext
	{
		// mapping of name to Group axis
		// these are groups of other axes so each can have its own processing
		// then be combined
		std::unordered_map<InputName, AxisGroup> GroupAxes;

		// mapping of name to axis
		// all buttons can be represented as axies with a deadzone
		// multibutton combos can be thought as an axis with components ('A', .5) and ('B', .5) and a dead zone = 1
		std::unordered_map<InputName, InputAxis> Axes;

		// mapping of code to inputname
		// allows us to skip a search through all components of each axis to find a mapping
		std::unordered_map<InputCode, InputName> Mapping;

		// raw states
		std::unordered_map<int, float> State;

		// for mapping mouse into viewport
		vec2 ViewportMin;
		vec2 ViewportSize;

		// active mask
		std::string activeMask;

		InputContext();
	};

	wContextDecl(InputContext);

	float GetButton(const InputName& button);
	vec2 GetAxis(const InputName& axis);

	bool AxisExists(const InputName& axis);
	bool GroupAxisExists(const InputName& axis);

	//
	//	Creating axes
	//

	InputAxis& CreateAxis(const InputName& name);
	AxisGroup& CreateGroupAxis(const InputName& name);

	//
	//	Settings
	//

	void SetAxisSettings(const InputName& axis, const InputAxisSettings& settings);
	void SetGroupAxisSettings(const InputName& axis, const InputAxisSettings& settings);

	//
	//	Components
	//

	void SetAxisComponent(const InputName& axis, InputCode code, vec2 weight = vec2(1, 0));

	// Combine multiple axes into a single Group axis
	void SetGroupAxisComponent(const InputName& axis, const InputName& component);
	
	//
	//	Mappings
	//

	const InputName& GetMapping(InputCode code);

	// Internal, for framework to set state
	void SetState(int code, float state);

	//
	// Save an input context to disk and load it
	//

	void WriteInputPack(const std::string& filename);
	void  ReadInputPack(const std::string& filename);

	//
	//	Viewport
	//

	void SetViewportBounds(vec2 screenMin, vec2 screenSize);
	vec2 MapToViewport(float screenX, float screenY);

	//
	//	Mask
	//

	void SetActiveMask(const std::string& mask);
}