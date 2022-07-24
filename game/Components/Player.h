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
	int Ammo = 5;
	int AmmoMax = 5;
	float AmmoRechargeTimer = 0.f;
	float AmmoRechargeTime = .8f;

	float MovementAccelerationScaleFactor = 15.f;
	float MovementSpeed = 35.f;// 40.f;
	float RotationSpeed = wPI * 1.2f;

	//vec2 AttackDirectionInput;

	float AttackFuelAlt; // gets incremented in the UI system :yuk: and decremented in the player system
	float AttackFuelConsumptionAlt = .05;
	float AttackFuelAdditionAlt = .1;

	WeaponProps Current;
	WeaponProps Alt;

	bool HasDied = false;

	float m_attackTimer = 0.f;
	float m_attackTimerAlt = 0.f;
};