#pragma once

#include "Common.h"

struct Player
{
	vec2 MovementInput;
	float MovementAccelerationScaleFactor = 15.f;
	float MovementSpeed = 40.f;
};