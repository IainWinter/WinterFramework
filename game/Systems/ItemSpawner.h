#pragma once

#include "Leveling.h"
#include "Sand/SandEvents.h"
#include "Sand/SandHelpers.h"
#include "Events.h"

struct System_ItemSpawner : System<System_ItemSpawner>
{
	void Init()
	{
		Attach<event_Sand_RemoveCell>();
	}

	void on(event_Sand_RemoveCell& e)
	{
		CorePixels p = GetCorePixels(e.entity.Get<Sprite>().source);
		if (std::find(p.core.begin(), p.core.end(), e.index) != p.core.end())
		{
			Send(event_Item_Spawn{ ITEM_CORE_SHARD, e.hitPosInWorld });
		}
	}
};