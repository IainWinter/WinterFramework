#pragma once

#include "Leveling.h"
#include "Events.h"

#include "Rendering.h"
#include "Prefabs.h"

#include "ext/Time.h"

struct System_Item : System<System_Item>
{
	void Init()
	{
		Attach<event_Item_Spawn>();
	}

	void Update()
	{
		for (auto [sinkEntity, sinkTransform, _] : QueryWithEntity<Transform2D, ItemSink>())
		{
			for (auto [itemEntity, itemTransform, item] : QueryWithEntity<Transform2D, Item>())
			{
				vec2 delta = itemTransform.position - sinkTransform.position;
				float dist = length(delta);
				
				if (item.m_hasSink) // pickup item
				{
					item.m_timeToSink += Time::DeltaTime();

					if (item.m_timeToSink >= 1.0f)
					{
						Send(event_Item_Pickup{ sinkEntity, item.type });
						itemEntity.Destroy();
					}

					else
					{
						itemTransform.position = lerp(item.m_initPosSink, sinkTransform.position, item.m_timeToSink);
					}
				}

				else              // set the picking up flag
				{
					if (dist < item.pickupRadius)
					{
						item.m_hasSink = true;
						item.m_initPosSink = itemTransform.position;
					}
				}
			}
		}
	}

	void on(event_Item_Spawn& e)
	{
		r<Texture> itemTexture;

		switch (e.type)
		{
			case ItemType::HEALTH: GetPrefab_Texture();
		}

		for (int i = 0; i < e.count; i++)
		{
			Entity entity = CreateEntity();
			entity.Add<Transform2D>(e.pos);
			entity.Add<Item>(e.type);
			



		}
	}
};