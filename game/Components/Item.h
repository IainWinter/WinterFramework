#pragma once

enum ItemType
{
	HEALTH,
	LASER
};

struct Item
{
	ItemType type;
	float pickupRadius = 2.f;

	bool  m_hasSink = false;
	float m_timeToSink = 1.f;  // has to be between 0-1 for lerp
	vec2  m_initPosSink;        // once hasSink has been set, this is the inital position of the item
};

struct ItemSink
{
	int _pad;
};