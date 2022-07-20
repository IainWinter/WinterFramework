#pragma once

#include "SDL2/SDL_scancode.h"
#include <unordered_map>

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

	ATTACK,
	ATTACK_ALT,

	ESCAPE
};

struct event_Input
{
	InputName name;
	float state;
};

struct InputMap
{
	std::unordered_map<SDL_Scancode, InputName> Keyboard;

	InputName Map(SDL_Scancode code) const 
	{
		auto itr = Keyboard.find(code);
		if (itr != Keyboard.end()) return itr->second;
		return InputName::_NONE;
	}

	void Set(SDL_Scancode code, InputName name)
	{
		Keyboard[code] = name;
	}
};