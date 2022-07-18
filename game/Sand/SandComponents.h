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
	u32   owner = 0u; // for no damage to who fired
	float health = 15; // how many cells can this projectile destroy
	float turnOnHitRate = .2f; // percent of velocity that should be used to turn on each cell hit
	float trailLife = .02;
	int   size = 0;

	CellProjectile& SetOwner     (u32   owner)         { this->owner = owner;              return *this; }
	CellProjectile& SetHealth    (float health)        { this->health = health;               return *this; }
	CellProjectile& SetTurnRate  (float turnOnHitRate) { this->turnOnHitRate = turnOnHitRate; return *this; }
	CellProjectile& SetTrailLife (float trailLife)     { this->trailLife = trailLife;         return *this; }
	CellProjectile& SetSize      (int   size)          { this->size = size;                   return *this; }
};

struct CorePixels
{
private:
	std::unordered_set<int> core; // should use unordered sets
	std::unordered_set<int> all;

public:
	size_t CoreSize()  const { return core.size(); }
	size_t TotalSize() const { return all.size(); }
	bool   HasCore()   const { return CoreSize () > 0; }
	bool   HasAny()    const { return TotalSize() > 0; }

	bool IsInCore(int index) const { return core.find(index) != core.end(); }
	bool IsInAll (int index) const { return all .find(index) != all .end(); }

	std::vector<int> GetCoreAsVec() const { return std::vector<int>(core.begin(), core.end()); }
	std::vector<int> GetAllAsVec()  const { return std::vector<int>(all .begin(), all .end()); }
	
	const std::unordered_set<int>& GetCore() const { return core; }
	const std::unordered_set<int>& GetAll()  const { return all; }


	void Add(int index, bool isCore)
	{
		all.insert(index);
		if (isCore) core.insert(index);
	}

	void Remove(int index)
	{
		if (IsInCore(index)) core.erase(index);
		if (IsInAll (index)) all .erase(index);
	}
};

struct SandSprite
{
	float density = 100.f;
	float cellStrength = 1.f;
	bool invulnerable = false;
	r<Texture> colliderMask;
	std::vector<int> initalCore;
	CorePixels pixels;

	bool isCircle = false;
	bool isHardCore = false;

	int CellCount() const { return pixels.TotalSize(); }
	Texture& Get() { return *colliderMask; }

	SandSprite() = default;
	SandSprite(const Texture& collider) : colliderMask(mkr<Texture>(collider)) {}
	SandSprite(r<Texture> collider)     : colliderMask(collider) {}


	SandSprite& SetDensity       (float density)      { this->density = density;           return *this; }
	SandSprite& SetCellStrength  (float cellStrength) { this->cellStrength = cellStrength; return *this; }
	SandSprite& SetIsInvulnerable(bool invulnerable)  { this->invulnerable = invulnerable; return *this; }
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