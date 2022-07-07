#pragma once

#include "Entity.h"
#include "ext/rendering/Sprite.h"
#include "Sand/SandComponents.h"
#include <functional>
#include <vector>

struct event_SandCellCollision
{
	Entity entity;
	Entity projectile;
	ivec2 hitPosInSprite;
};

// cretea a mesh collider is entity has no Rigidbody component
struct event_SandAddSprite
{
	Entity entity;
	vec2 velocity = vec2(0.f, 0.f);
	float aVelocity = 0.f;
	float density = 1.f;
};

//struct event_Sand_CreateCell
//{
//	CellDefinition cell;
//	std::function<void(Entity)> onCreate;
//
//	event_Sand_CreateCell(
//		CellDefinition cell,
//		std::function<void(Entity)> onCreate = {}
//	)
//		: cell           (cell)
//		, onCreate       (onCreate)
//	{}
//
//	event_Sand_CreateCell(
//		vec2 pos, 
//		vec2 vel, 
//		Color color, 
//		float life,
//		std::function<void(Entity)> onCreate = {}
//	)
//		: cell           (CellDefinition{ pos, vel, color, life })
//		, onCreate       (onCreate)
//	{}
//};

struct event_Sand_CreateCollider
{
	Entity entity; // requires Rigidbody2D, SandSprite
};

struct event_Sand_ExplodeToDust
{
	EntityWith<Transform2D, Sprite, SandSprite> entity;
	std::vector<int> onlyThisIndex; // if empty everything is blow
	vec2 velocity;
	
	//vec2 projectileVel;
};

struct event_Sand_ProjectileHit
{
	EntityWith<CellProjectile> projectile;
	EntityWith<Transform2D, Sprite, SandSprite> entity;
	int index;
	ivec2 hitPosInSprite; // in pixels
	vec2 hitPosInWorld;   // in meters
};

struct event_Sand_RemoveCell
{
	EntityWith<Sprite, SandSprite> entity;
	int index;
	vec2 hitPosInWorld;
};

struct event_Sand_HealCell
{
	EntityWith<Sprite, SandSprite, SandHealable> entity;
	int index;
	bool healCore = false;
};