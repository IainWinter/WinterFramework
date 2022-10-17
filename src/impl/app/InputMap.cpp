#include "app/InputMap.h"

namespace Input
{
	struct InputAxis
	{
		std::unordered_map<int, vec2> components; // sum of State[code] * component is axis state
		float deadzone = 0.f;
	};

	struct VirtualAxis
	{
		std::vector<InputName> children;
	};

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

	float GetButton(InputName button)
	{
		return GetAxis(button).x;
	}

	vec2 GetAxisNoRecurse(InputName axis)
	{
		vec2 out = vec2(0.f);

		auto itr = Axes.find(axis);
		if (itr != Axes.end())
		{
			for (const auto& component : itr->second.components)
			{
				out += State[component.first] * component.second;
			}

			if (length(out) < itr->second.deadzone)
			{
				out = vec2(0.f);
			}
		}

		return limit(out, 1.f);
	}

	vec2 GetAxis(InputName axis)
	{
		vec2 out = GetAxisNoRecurse(axis);

		auto vitr = VirtualAxes.find(axis);
		if (vitr != VirtualAxes.end())
		{
			for (const auto& child : vitr->second.children)
			{
				out += GetAxisNoRecurse(child);
			}
		}

		return limit(out, 1.f);
	}

	void CreateAxis(InputName name)
	{
		if (AxisExists(name))
		{
			log_app("w~Axis already exists. %s", name);
			return;
		}

		Axes.emplace(name, InputAxis{});
	}

	void CreateVirtualAxis(InputName name)
	{
		if (VirtualAxisExists(name))
		{
			log_app("w~Virtual axis already exists. %s", name);
			return;
		}

		VirtualAxes.emplace(name, VirtualAxis{});
	}

	bool AxisExists(InputName axis)
	{
		return Axes.find(axis) != Axes.end();
	}

	bool VirtualAxisExists(InputName axis)
	{
		return VirtualAxes.find(axis) != VirtualAxes.end();
	}

	void SetDeadzone(InputName axis, float deadzone)
	{
		if (!AxisExists(axis))
		{
			log_app("w~Axis doesn't exist. %s", axis);
			return;
		}

		Axes[axis].deadzone = deadzone;
	}

	void SetVirtualAxisComponent(InputName axis, InputName component)
	{
		if (!VirtualAxisExists(axis))
		{
			log_app("w~Virtual axis doesn't exist. %s", axis);
			return;
		}

		if (!AxisExists(component))
		{
			log_app("w~Axis doesn't exist. %s", axis);
			return;
		}

		VirtualAxes[axis].children.push_back(component);

		for (const auto& c : Axes[component].components) // we need to update mapping 
		{
			Mapping[c.first] = axis;
		}
	}

	// translation helpers

	int GetCode(KeyboardInput scancode)
	{
		return (int)scancode;
	}

	int GetCode(ControllerInput input)
	{
		return 1 + (int)input + SDL_NUM_SCANCODES;
	}

	InputName GetMapping(int code)
	{
		auto itr = Mapping.find(code);
		return itr != Mapping.end() ? itr->second : nullptr;
	}

	InputName GetMapping(KeyboardInput scancode)
	{
		return GetMapping(GetCode(scancode));
	}

	InputName GetMapping(ControllerInput scancode)
	{
		return GetMapping(GetCode(scancode));
	}

	void SetAxisComponent(InputName axis, int code, vec2 weight)
	{
		if (!AxisExists(axis))
		{
			log_app("w~Axis doesn't exist. %s", axis);
			return;
		}

		Axes[axis].components.emplace(code, weight);
		Mapping.emplace(code, axis);
	}

	void SetAxisComponent(InputName axis, KeyboardInput scancode, vec2 weight)
	{
		SetAxisComponent(axis, GetCode(scancode), weight);
	}

	void SetAxisComponent(InputName axis, ControllerInput input, vec2 weight)
	{
		SetAxisComponent(axis, GetCode(input), weight);
	}

	// internal

	void SetState(int code, float state)
	{
		if (code < 0 || code >= NUMBER_OF_STATES)
		{
			log_app("e~Tried to set state of code that is invalid. %d -> %f", code, state);
			return;
		}

		State[code] = state;
	}
}