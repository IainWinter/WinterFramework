#pragma once

#include "Common.h"
#include "Rendering.h"

struct Cell
{
	vec2 pos;
	vec2 vel;
	Color color;
	float life = 0.f;
};

struct CellProjectile
{
	u32 owner; // for no damage to who fired
	int health = 25; // how many cells can this projectile destroy
	float turnOnHitRate = .2f; // percent of velocity that should be used to turn on each cell hit
};

struct SandSprite
{
	float density = 100.f;
	bool invulnerable = false;
	r<Texture> colliderMask;
	std::vector<int> core; // if this list has items, these are the only cells to floodfill
	int hasCore = false;  // if the list is health or every pixel
	//int originalCoreCount = 0;

	int cellStrength = 1;
	int cellCount = 0;

	Texture& Get() { return *colliderMask; }

	SandSprite() = default;
	SandSprite(const Texture& collider) : colliderMask(mkr<Texture>(collider)) {}
	SandSprite(r<Texture> collider)     : colliderMask(collider) {}
};