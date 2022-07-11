#pragma once

#include "Common.h"
#include "Components/Weapon.h"

struct WeaponProps
{
	Weapon Weapon;
	int Ammo;
	float Inaccuracy;
	float AttackTime;
};

struct Player
{
	vec2 MovementInput;
	float MovementAccelerationScaleFactor = 15.f;
	float MovementSpeed = 40.f;

	//vec2 AttackDirectionInput;
	vec2 AttackLocationInput;
	bool AttackFireInput;
	bool AttackFireInputAlt;

	float AttackFuelAlt; // gets incremented in the UI system :yuk: and decremented in the player system
	float AttackFuelConsumptionAlt = .05;
	float AttackFuelAdditionAlt = .1;

	WeaponProps Current;
	WeaponProps Alt;

	int Score = 0; // storing this here for simplicity

	float m_attackTimer = 0.f;
	float m_attackTimerAlt = 0.f;
};