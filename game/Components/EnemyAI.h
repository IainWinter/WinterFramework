#pragma once

#include "Entity.h"

enum Weapon
{
	LASER
};

struct TurnTwoardsTarget
{
	Entity target; // only needs Transform2D, could just be an empty point that only has a Transform2D
	float strength = 0.9f;
	float atTargetVelocityDampen = 1.f;
};

struct FireWeaponAfterDelay
{
	Entity target;
	Weapon weapon;
	float delay;
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