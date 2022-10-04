#include "app/InputMap.h"

namespace Input
{
	std::unordered_map<SDL_Scancode, InputName> Keyboard;
	std::unordered_map<InputName, float> State;

	InputName Map(SDL_Scancode code)
	{
		auto itr = Keyboard.find(code);
		if (itr != Keyboard.end()) return itr->second;
		return InputName::_NONE;
	}

	void Set(SDL_Scancode code, InputName name)
	{
		Keyboard[code] = name;

	}

	void SetMap(const std::unordered_map<SDL_Scancode, InputName>& map)
	{
		Keyboard = map;
	}

	float GetState(InputName name)
	{
		auto itr = State.find(name);
		if (itr != State.end()) return itr->second;
		return 0.f;
	}

	void SetState(InputName name, float state)
	{
		State[name] = state;
	}
}