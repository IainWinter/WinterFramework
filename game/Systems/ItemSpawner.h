#pragma once

#include "app/System.h"
#include "Sand/SandEvents.h"
#include "Sand/SandHelpers.h"
#include "Events.h"
#include "Components/Player.h"

struct System_ItemSpawner : System<System_ItemSpawner>
{
	Entity playerEntity;

	void Init()
	{
		Attach<event_Sand_RemoveCell>();
		playerEntity = FirstEntity<Player>();
	}

	void on(event_Sand_RemoveCell& e)
	{
		if (e.entity == playerEntity)
		{
			if (e.entity.Get<SandSprite>().pixels.IsInCore(e.index))
			{
				Send(event_Item_Spawn{ ITEM_CORE_SHARD, e.hitPosInWorld });
			}
		}
	}
};