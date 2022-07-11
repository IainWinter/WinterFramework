#pragma once

enum ItemType
{
	ITEM_HEALTH,
	ITEM_ENERGY,
	ITEM_REGOLITH,

	ITEM_CORE_SHARD,
	ITEM_WEAPON_CANNON,
	ITEM_WEAPON_BOLTZ,
	ITEM_WEAPON_WATTZ,
	ITEM_WEAPON_MINIGUN,
};

struct Item
{
	ItemType type;
	float pickupRadius = 2.f;
	float pickupDelay = .2f;
	float life = 4.f;

	bool  m_hasSink = false;
	float m_timeToSink = 1.f;  // has to be between 0-1 for lerp
	vec2  m_initPosSink;        // once hasSink has been set, this is the inital position of the item
	vec2  m_initScale;
};

struct ItemSink
{
	int _pad;
};