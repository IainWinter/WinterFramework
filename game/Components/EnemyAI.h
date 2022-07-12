#pragma once

#include "Entity.h"
#include "Components/Weapon.h"
#include "Components/Item.h"

enum EnemyType
{
	ENEMY_FIGHTER,
	ENEMY_BOMB,
	ENEMY_STATION,
	ENEMY_BASE
};

struct Enemy
{
	int itemGas;
};

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
	float m_timer = 3.f;
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
	struct LocalSpawnPoint
	{
		vec2 point;
		EnemyType type;
	};

	std::vector<LocalSpawnPoint> spawners;
	float delay = 2.f;
	float timer = 0.f;

	EnemySpawner& AddSpawner (const LocalSpawnPoint& spawner)               { spawners.push_back(spawner); return *this; }
	EnemySpawner& SetSpawners(const std::vector<LocalSpawnPoint>& spawners) { this->spawners = spawners;   return *this; }
	EnemySpawner& SetDelay   (float delay)                                  { this->delay = delay;         return *this; }
};