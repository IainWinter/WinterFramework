#pragma once

#include "Common.h"

struct Player
{
	vec2 MovementInput;
	float MovementAccelerationScaleFactor = 15.f;
	float MovementSpeed = 40.f;

	//vec2 AttackDirectionInput;
	vec2 AttackLocationInput;
	float AttackTime = .4f;
	float m_attackTimer = 0.f;
	bool AttackFireInput;
};