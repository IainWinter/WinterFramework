#include "app/InputMap.h"

namespace Input
{
	struct InputAxis
	{
		std::unordered_map<int, vec2> components; // sum of State[code] * component is axis state
		float deadzone;
	};

	// mapping of name to axis
	// all buttons can be represented as axies with a deadzone
	// multibutton combos can be thought as a axis with components ('A', .5) and ('B', .5) and a dead zone = 1
	std::unordered_map<InputName, InputAxis> Axies;

	// mapping of code to inputname
	// allows us to skip a search through all components of each axis to find a mapping
	std::unordered_map<int, InputName> Mapping;

	// raw states
	std::array<float, NUMBER_OF_STATES> State;

	float GetButton(InputName button)
	{
		return GetAxis(button).x;
	}

	vec2 GetAxis(InputName axis)
	{
		vec2 out = vec2(0.f);

		auto itr = Axies.find(axis);
		if (itr != Axies.end())
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

	void SetDeadzone(InputName axis, float deadzone)
	{
		Axies[axis].deadzone = deadzone;
	}

	InputName _GetMapping(int code)
	{
		auto itr = Mapping.find(code);
		return itr != Mapping.end() ? itr->second : InputName::_NONE;
	}

	void _SetMapping(InputName name, int code)
	{
		_SetAxisComponent(name, code, vec2(1.f, 0.f));
	}

	void _SetAxisComponent(InputName axis, int code, vec2 component)
	{
		Axies[axis].components.emplace(code, component);
		Mapping.emplace(code, axis);
	}

	void _SetState(int code, float state)
	{
		State[code] = state;
	}

	// translation helpers

	int GetCode(KeyboardInput scancode)
	{
		return (int)scancode;
	}

	int GetCode(ControllerInput button)
	{
		return 1 + (int)button + SDL_NUM_SCANCODES;
	}

	InputName GetMapping(KeyboardInput scancode)
	{
		return _GetMapping(GetCode(scancode));
	}

	void SetMapping(InputName name, KeyboardInput scancode)
	{
		_SetMapping(name, GetCode(scancode));
	}

	void SetAxisComponent(InputName axis, KeyboardInput scancode, vec2 component)
	{
		_SetAxisComponent(axis, GetCode(scancode), component);
	}

	InputName GetMapping(ControllerInput scancode)
	{
		return _GetMapping(GetCode(scancode));
	}

	void SetMapping(InputName name, ControllerInput button)
	{
		_SetMapping(name, GetCode(button));
	}

	void SetAxisComponent(InputName axis, ControllerInput button, vec2 component)
	{
		_SetAxisComponent(axis, GetCode(button), component);
	}
}