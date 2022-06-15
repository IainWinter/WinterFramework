#pragma once

#include "Common.h"
#include "Rendering.h"

struct Cell
{
	vec2 pos;
	vec2 vel;
	float dampen = 20;
	Color color;
};

struct CellProjectile
{
	u32 owner; // for no damage to who fired
	int health = 15; // how many cells can this projectile destroy
};

struct CellLife
{
	float life;
};

struct SandSprite
{
	float density = 100.f;
	bool invulnerable = false;
	r<Texture> colliderMask;
	std::vector<int> core; // if this list has items, these are the only cells to floodfill
	int hasCore = false;  // if the list is health or every pixel
	//int originalCoreCount = 0;

	int cellStrength = 0;
	int cellCount = 0;

	Texture& Get() { return *colliderMask; }

	SandSprite() = default;
	SandSprite(const Texture& collider) : colliderMask(mkr<Texture>(collider)) {}
	SandSprite(r<Texture> collider)     : colliderMask(collider) {}
};