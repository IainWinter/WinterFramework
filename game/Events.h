#pragma once

#include "Common.h"
#include "Entity.h"
#include "Components/Weapon.h"
#include "Components/Item.h"
#include "Components/EnemyAI.h"

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
	float inaccuracy;
};

struct event_Item_Spawn
{
	ItemType type;
	vec2 pos;
	int count = 1;
};

struct event_Item_Pickup
{
	Entity sinkEntity;
	ItemType item;
};

struct event_Enemy_Spawn
{
	EnemyType enemy;
	vec2 position;
	bool enableAi;
};