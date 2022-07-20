#pragma once

#include "app/System.h"
#include "Events.h"

struct System_ItemPickup : System<System_ItemPickup>
{
	void Init()
	{
		Attach<event_Item_Pickup>();
	}

	void on(event_Item_Pickup& e)
	{
		switch (e.item)
		{
			case ITEM_CORE_SHARD: {
				Send(event_Sand_HealCell{ e.sinkEntity, -1, true });
				break;
			}
		}
	}
};