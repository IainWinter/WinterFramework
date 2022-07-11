#pragma once

#include "Leveling.h"
#include "Sand/SandEntity.h"

#include "Components/EnemyAI.h"
#include "Components/Flocker.h"
#include "Components/Player.h"

#include "util/random.h"

struct System_Enemy : System<System_Enemy>
{
	void Init()
	{
		Attach<event_Enemy_Spawn>();
	}

	void on(event_Enemy_Spawn& e)
	{
		SpawnEnemy(e.enemy, e.position, e.enableAi);
	}

private:

	void SpawnEnemy(EnemyType type, vec2 position, bool enableAI)
	{
		Entity entity;
		
		switch (type)
		{
			case ENEMY_FIGHTER:
			{
				entity = CreateSandSprite("enemy_fighter.png", "enemy_fighter_mask.png");
				entity.Add<Enemy>(10 + get_rand(10));

				if (enableAI)
				{
					entity.Add<FireWeaponAfterDelay>(FirstEntityWith<Player>(), WEAPON_LASER, 1.f + get_randc(.3f));
					entity.Add<TurnTwoardsTarget>(CreateEntity().AddAll(Transform2D(get_randc(32), get_randc(18))));
					entity.Add<Flocker>();
				}

				break;
			}

			case ENEMY_BOMB:
			{
				entity = CreateSandSprite("enemy_bomb.png", "enemy_bomb_mask.png");
				entity.Add<Enemy>(20 + get_rand(20));

				if (enableAI)
				{
					entity.Add<TurnTwoardsTarget>(FirstEntityWith<Player>());
				}

				break;
			}

			case ENEMY_STATION:
			{
				entity = CreateSandSprite("enemy_station.png", "enemy_station_mask.png");
				entity.Add<Enemy>(100 + get_rand(100));

				if (enableAI)
				{
					entity.Add<TurnTwoardsTarget>(CreateEntity().AddAll(Transform2D(get_rand(32, 18))));
					entity.Add<Flocker>();
				}

				break;
			}

			case ENEMY_BASE:
			{
				entity = CreateSandSprite("enemy_base.png", "enemy_base_mask.png");
				entity.Add<Enemy>(1000 + get_rand(1000));

				if (enableAI)
				{
					entity.Add<TurnTwoardsTarget>(CreateEntity().AddAll(Transform2D(get_rand(32, 18))));
					entity.Add<Flocker>();
				}

				break;
			}
		}

		Transform2D& transform = entity.Get<Transform2D>();
		transform.position = position;

		entity.Add<Rigidbody2D>(transform).SetFixedRotation(true);// .SetVelocity(get_randn(5.f));

		entity.OnDestroy([this](Entity e)
		{
				std::vector<ItemSpawn> item_table =
				{
					{ ITEM_HEALTH, 5, 10 }
				};


			int itemGas = e.Get<Enemy>();

			std::vector<std::pair<ItemSpawn, float>> item_weights {
				{ {ITEM_HEALTH,         10}, 50 },
				{ {ITEM_ENERGY,         10}, 50 },
				{ {ITEM_WEAPON_MINIGUN,  1},  3 },
				{ {ITEM_WEAPON_BOLTZ,    1},  2 },
				{ {ITEM_WEAPON_WATTZ,    1},  2 },
			};

			ItemSpawn item = choose(item_weights);

			Send(event_Item_Spawn{ item.type, e.Get<Transform2D>().position, item.count });
		});
	}

	struct ItemSpawn
	{
		ItemType type;
		int count;
		int gas;
	};
};