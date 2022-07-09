#pragma once

#include "Leveling.h"
#include "Events.h"
#include "Rendering.h"
#include "Prefabs.h"
#include "CoordTranslation.h"
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
			for (auto [itemEntity, itemTransform, itemBody, item] : QueryWithEntity<Transform2D, Rigidbody2D, Item>())
			{
				vec2 delta = sinkTransform.position - itemTransform.position;
				float dist = length(delta);
				
				if (item.m_hasSink) // pickup item
				{
					item.m_timeToSink -= Time::DeltaTime();

					if (item.m_timeToSink <= 0.f)
					{
						Send(event_Item_Pickup{ sinkEntity, item.type });
						itemEntity.Destroy();
					}

					else
					{
						if (item.m_timeToSink < .5f)
						{
							itemTransform.scale = lerp(vec2(0, 0), item.m_initScale, 2.f * item.m_timeToSink);
						}

						// todo: get this to pull the items in reliably

						vec2 vel = itemBody.GetVelocity();
						vel += delta * (1.f - item.m_timeToSink + .2f);
						itemBody.SetVelocity(vel);
					}
				}

				else              // set the picking up flag
				{
					item.pickupDelay -= Time::DeltaTime();
					item.life -= Time::DeltaTime();

					if (item.life < 0)
					{
						itemEntity.Destroy();
					}

					else
					if (item.life < 1)
					{
						itemTransform.scale = lerp(vec2(0, 0), item.m_initScale, item.life);
					}

					else
					if (item.pickupDelay < 0.f && dist < item.pickupRadius)
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
		float damping = 5.f;

		switch (e.type)
		{
			case ItemType::ITEM_HEALTH:         itemTexture = GetPrefab_Texture("item_health.png");                   break;
			case ItemType::ITEM_ENERGY:         itemTexture = GetPrefab_Texture("item_energy.png");                   break;
			case ItemType::ITEM_REGOLITH:       itemTexture = GetPrefab_Texture("item_regolith.png");                 break;
			case ItemType::ITEM_CORE_SHARD:     itemTexture = GetPrefab_Texture("item_coreShard.png"); damping = 0.f; break;
			case ItemType::ITEM_WEAPON_MINIGUN: itemTexture = GetPrefab_Texture("item_minigun.png");                  break;
			case ItemType::ITEM_WEAPON_BOLTZ:   itemTexture = GetPrefab_Texture("item_boltz.png");                    break;
			case ItemType::ITEM_WEAPON_WATTZ:   itemTexture = GetPrefab_Texture("item_wattz.png");                    break;
		}

		for (int i = 0; i < e.count; i++)
		{
			Entity entity = CreateEntity();
			entity.Add<Transform2D>(vec3(e.pos, 10.f), itemTexture->Dimensions() * GetModule<CoordTranslation>().CellsToMeters);
			entity.Add<Item>(e.type);
			entity.Add<Sprite>(itemTexture);

			GetModule<PhysicsWorld>().AddEntity(entity)
				.SetVelocity(get_randn(12.f))
				.SetAngularVelocity(get_rand(w2PI))
				.SetDamping(damping)
				.SetAngularDamping(damping);

			entity.Get<Item>().m_initScale = entity.Get<Transform2D>().scale;
		}
	}
};