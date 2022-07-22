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
	float MovementSpeed = 35.f;// 40.f;
	float RotationSpeed = wPI * 1.2f;

	//vec2 AttackDirectionInput;
	vec2 AttackLocationInput;
	float AttackFireInput;
	float AttackFireInputAlt;

	float AttackFuelAlt; // gets incremented in the UI system :yuk: and decremented in the player system
	float AttackFuelConsumptionAlt = .05;
	float AttackFuelAdditionAlt = .1;

	WeaponProps Current;
	WeaponProps Alt;

	int Score = 0; // storing this here for simplicity
	int Lives = 3; // this isnt the count, actual count lives in the system this is just for the UI to read it
	bool HasDied = false;

	float m_attackTimer = 0.f;
	float m_attackTimerAlt = 0.f;
};