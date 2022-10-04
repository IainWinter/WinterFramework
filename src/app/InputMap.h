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

	bool enabled() { return state == 1.f; }
};

// only handles keyboard mappings right now
// in future should be able to map a name to an axis / mouse pos

namespace Input
{
	InputName Map(SDL_Scancode code);
	void Set(SDL_Scancode code, InputName name);
	void SetMap(const std::unordered_map<SDL_Scancode, InputName>& map);

	float GetState(InputName name);
	void  SetState(InputName name, float state);
}