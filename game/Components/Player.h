#pragma once

#include "Common.h"
#include "Components/Weapon.h"

struct Player
{
	vec2 MovementInput;
	float MovementAccelerationScaleFactor = 15.f;
	float MovementSpeed = 40.f;

	//vec2 AttackDirectionInput;
	vec2 AttackLocationInput;
	float m_attackTimer = 0.f;
	bool AttackFireInput;

	Weapon CurrentWeapon = WEAPON_CANNON;
	int CurrentWeaponAmmo = 0;
	float CurrentWeaponInaccuracy = 0;
	float CurrentWeaponAttackTime = .4f;

	int Score = 0; // storing this here for simplicity
};