#pragma once

enum ItemType
{
	ITEM_HEALTH,
	ITEM_ENERGY,
	ITEM_CORE_SHARD,

	ITEM_WEAPON_BOLTZ,
	ITEM_WEAPON_WATTZ,
	ITEM_WEAPON_MINIGUN
};

struct Item
{
	ItemType type;
	float pickupRadius = 2.f;

	bool  m_hasSink = false;
	float m_timeToSink = 0.f;  // has to be between 0-1 for lerp
	vec2  m_initPosSink;        // once hasSink has been set, this is the inital position of the item
	vec2  m_initScaleSink;
};

struct ItemSink
{
	int _pad;
};