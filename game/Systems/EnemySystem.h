#pragma once

#include "Leveling.h"
#include "Sand/SandEntity.h"

#include "Components/EnemyAI.h"
#include "Components/Flocker.h"
#include "Components/Player.h"
#include "Components/Throwable.h"

#include "util/random.h"

struct System_Enemy : System<System_Enemy>
{
	Entity player;

	void Init()
	{
		Attach<event_Enemy_Spawn>();
		player = FirstEntityWith<Player>();
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

				entity.Add<Rigidbody2D>()
					.SetFixedRotation(true)
					.SetDensity(30.f);

				if (enableAI)
				{
					entity.Add<FireWeaponAfterDelay>(player, WEAPON_LASER, 1.f + get_randc(.3f));
					entity.Add<TurnTwoardsTarget>(CreateEntity().AddAll(Transform2D(get_randc(32), get_randc(18))));
					entity.Add<Flocker>()
						.SetMaxSpeed(15.f);
				}

				break;
			}

			case ENEMY_BOMB:
			{
				entity = CreateSandSprite("enemy_bomb.png", "enemy_bomb_mask.png");
				entity.Add<Enemy>(20 + get_rand(20));

				entity.Add<Rigidbody2D>()
					.SetFixedRotation(false)
					.SetVelocity(vec2(5.f))
					.SetAngularVelocity(get_randc(wPI / 3.f))
					.SetAngularDamping(0.5f)
					.SetDensity(10.f);

				if (enableAI)
				{
					entity.Add<TurnTwoardsTarget>(player);
					entity.Add<ExplodeNearTarget>(player);

					entity.Add<Throwable>();
				}

				break;
			}

			case ENEMY_STATION:
			{
				entity = CreateSandSprite("enemy_station.png", "enemy_station_mask.png");
				entity.Add<Enemy>(100 + get_rand(100));

				entity.Add<Rigidbody2D>()
					.SetFixedRotation(false)
					.SetAngularDamping(1.f)
					.SetDensity(400.f);

				if (enableAI)
				{
					entity.Add<TurnTwoardsTarget>(CreateEntity().AddAll(Transform2D(get_randc(32, 18)))); // leaks entity
					
					entity.Add<Flocker>()
						.SetMaxSpeed(5.f);

					entity.Add<EnemySpawner>()
						.SetDelay(4.f)
						.SetSpawners({
							{ vec2( 1,  0), ENEMY_BOMB },
							{ vec2( 1,  0), ENEMY_FIGHTER },
							{ vec2(-1,  0), ENEMY_BOMB },
							{ vec2(-1,  0), ENEMY_FIGHTER },
							{ vec2( 0,  1), ENEMY_BOMB },
							{ vec2( 0,  1), ENEMY_FIGHTER },
							{ vec2( 0, -1), ENEMY_BOMB },
							{ vec2( 0, -1), ENEMY_FIGHTER },
						});
				}

				break;
			}

			case ENEMY_BASE:
			{
				entity = CreateSandSprite("enemy_base.png", "enemy_base_mask.png");
				entity.Add<Enemy>(200 + get_rand(200));

				entity.Add<Rigidbody2D>()
					.SetFixedRotation(false)
					.SetAngularDamping(1.f)
					.SetDensity(1000.f);

				if (enableAI)
				{
					entity.Add<TurnTwoardsTarget>(CreateEntity().AddAll(Transform2D(get_randc(32, 18)))); // leaks entity
					
					entity.Add<Flocker>()
						.SetMaxSpeed(3.f);

					entity.Add<Thrower>()
						.SetTarget(player);
				}

				break;
			}
		}

		float rot = entity.Get<Rigidbody2D>().IsFixedRotation() ? 0 : get_rand(w2PI);

		entity.Get<Transform2D>().SetPosition(position).SetRotation(rot);
		entity.Get<Rigidbody2D>().SetTransform(entity.Get<Transform2D>());

		entity.OnDestroy([this](Entity e)
		{
			std::vector<std::pair<ItemSpawn, float>> item_weights {
				{ {ITEM_REGOLITH,        1,   1}, 100 },
				{ {ITEM_HEALTH,          1,  10},  60 },
				{ {ITEM_ENERGY,          1,  10},  50 },
				{ {ITEM_WEAPON_MINIGUN,  1, 100},  20 },
				{ {ITEM_WEAPON_BOLTZ,    1, 180},  20 },
				{ {ITEM_WEAPON_WATTZ,    1, 200},  20 },
			};

			int gas = e.Get<Enemy>().itemGas;

			while (gas > 1)
			{
				size_t itemIndex = choosei(item_weights);
				ItemSpawn& item = item_weights.at(itemIndex).first;
				
				if (item.gas > gas)
				{
					item_weights.erase(item_weights.begin() + itemIndex);
					continue;
				}

				gas -= item.gas;

				Send(event_Item_Spawn{ item.type, e.Get<Transform2D>().position, item.count });
			}
		});
	}

	struct ItemSpawn
	{
		ItemType type;
		int count;
		int gas;
	};
};