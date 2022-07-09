#pragma once

#include "Leveling.h"
#include "Sand/SandEvents.h"
#include "Sand/SandHelpers.h"
#include "Events.h"

struct System_ItemSpawner : System<System_ItemSpawner>
{
	Entity playerEntity;

	void Init()
	{
		Attach<event_Sand_RemoveCell>();
		playerEntity = FirstEntityWith<Player>();
	}

	void on(event_Sand_RemoveCell& e)
	{
		if (e.entity == playerEntity)
		{
			const CorePixels& p = e.entity.Get<SandSprite>().pixels;
			if (std::find(p.core.begin(), p.core.end(), e.index) != p.core.end())
			{
				Send(event_Item_Spawn{ ITEM_CORE_SHARD, e.hitPosInWorld });
			}
		}
	}
};