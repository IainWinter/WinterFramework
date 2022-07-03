#pragma once

#include "Entity.h"
#include "Components/Weapon.h"

struct TurnTwoardsTarget
{
	EntityWith<Transform2D> target; // only needs Transform2D, could just be an empty point that only has a Transform2D
	float strength = 0.6f;
	float atTargetVelocityDampen = .1f;
};

struct FireWeaponAfterDelay
{
	Entity target;
	Weapon weapon;
	float delay;
	float inaccuracy;
	float m_timer;
};

struct ExplodeNearTarget
{
	Entity target;
	float explosionPower = 200;
	float distanceToStartFuse = 5.f;
	float fuse = 2.f;
	bool m_tickFuse = false;
	bool m_exploded = false;
};

struct EnemySpawner
{
	//
};