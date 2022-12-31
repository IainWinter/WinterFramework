#pragma once

#include "Common.h"
#include "util/context.h"
#include <unordered_map>
#include <array>

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
	struct InputAxis
	{
		std::unordered_map<int, vec2> components; // sum of State[code] * component is axis state
		float deadzone = 0.f;
		bool limitToUnit = true;

		bool operator==(const InputAxis& other) const {
			return deadzone == other.deadzone 
				&& std::equal(components.begin(), components.end(), other.components.begin());
		}

		bool operator!=(const InputAxis& other) const {
			return !operator==(other);
		}
	};

	struct VirtualAxis
	{
		std::vector<InputName> axes;
		bool limitToUnit = true;

		bool operator==(const VirtualAxis& other) const {
			return std::equal(axes.begin(), axes.end(), other.axes.begin());
		}

		bool operator!=(const VirtualAxis& other) const {
			return !operator==(other);
		}
	};

	struct InputContext : wContext
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
		std::unordered_map<int, float> State;

		// for mapping mouse into viewport
		vec2 ViewportMin;
		vec2 ViewportSize;

		InputContext();
	};

	wContextDecl(InputContext);

	float GetButton(const InputName& button);
	vec2 GetAxis(const InputName& axis);

	void CreateAxis(const InputName& name);
	void CreateVirtualAxis(const InputName& name);

	bool AxisExists(const InputName& axis);
	bool VirtualAxisExists(const InputName& axis);

	// Set the deadzone of an axis. Not a virtual aixs, use this for the components
	void SetDeadzone(const InputName& axis, float deadzone);

	// set an inputs weight on an axis from its code, see GetInputCode
	void SetAxisComponent(const InputName& axis, int code, vec2 weight = vec2(1.f, 0.f));
	
	// Set a key's weight on an axis
	void SetAxisComponent(const InputName& axis, KeyboardInput scancode, vec2 weight = vec2(1.f, 0.f));
	
	// Set a controller input's weight on an axis
	void SetAxisComponent(const InputName& axis, ControllerInput input, vec2 weight = vec2(1.f, 0.f));

	// Combine multiple axes into a single virtual axis
	void SetVirtualAxisComponent(const InputName& axis, const InputName& component);
	
	const InputName& GetMapping(int code);
	const InputName& GetMapping(KeyboardInput scancode);
	const InputName& GetMapping(ControllerInput input);

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
}