#pragma once

#include "Common.h"
#include "Weapons.h"

struct event_SpawnExplosion
{
	vec2 position;
	float power;
};

struct event_FireWeapon
{
	Entity owner;
	Entity target;
	Weapon type;
};