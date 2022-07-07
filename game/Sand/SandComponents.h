#pragma once

#include "Common.h"
#include "Rendering.h"
#include <vector>

struct CellDefinition
{
	vec2 pos;
	vec2 vel;
	Color color;
	float life = -1.f; // negitive lifes forever
};

struct CellProjectile
{
	u32 owner; // for no damage to who fired
	int health = 5; // how many cells can this projectile destroy
	float turnOnHitRate = .2f; // percent of velocity that should be used to turn on each cell hit
	float trailLife = .02;
	int size = 0;
};

struct CorePixels
{
	std::vector<int> core;
	std::vector<int> all;
	bool hasCore;
	bool hasAny;
};

struct SandSprite
{
	float density = 100.f;
	bool invulnerable = false;
	r<Texture> colliderMask;
	std::vector<int> initalCore;
	int cellStrength = 0;
	bool isCircle = false;
	CorePixels pixels;

	int CellCount() const { return pixels.all.size(); }
	Texture& Get() { return *colliderMask; }

	SandSprite() = default;
	SandSprite(const Texture& collider) : colliderMask(mkr<Texture>(collider)) {}
	SandSprite(r<Texture> collider)     : colliderMask(collider) {}
};

struct SandHealable
{
	std::vector<int> removedFromCore;
	std::vector<int> removedFromShell;
};

struct SandTurnToDustInTime
{
	float time;
};

struct SandDieInTimeWithLowCoreCount
{
	float initalTimeToDie = 4.f;
	float jitterInMeters = 1.f;
	
	float m_timeToDie;
	bool m_runTimer = false;
};