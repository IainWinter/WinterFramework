#pragma once

#include "Entity.h"
#include "Common.h"
#include <functional>

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

struct event_SandTurnSpriteToDust
{
	Entity entity;
};

struct event_SpawnSandCell
{
	vec2 position;
	vec2 velocity;
	Color color;

	float life = 0.f;

	std::function<void(Entity)> onCreate;
};

struct event_Sand_CreateCollider
{
	Entity entity; // requires Rigidbody2D, SandSprite
};