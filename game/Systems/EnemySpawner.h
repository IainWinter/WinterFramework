#pragma once

#include "Leveling.h"
#include "ext/rendering/Camera.h"
#include "ext/Time.h"
#include "Sand/SandEntity.h"

#include "Components/EnemyAI.h"
#include "Components/Flocker.h"
#include "Components/Player.h"

struct System_EnemySpawner : SystemBase
{
	struct SpawningZone
	{
		vec2 min;
	};

	std::vector<SpawningZone> m_zones;
	vec2 m_zoneSize;

	void Init()
	{
		float padding = 0.f;
		int numberOfZones = 5;
		vec2 screenSize = GetModule<Camera>().ScreenSize();
		
		m_zoneSize = screenSize * 2.f / (float)numberOfZones; // zones are square so x and w/h of zones on x and y is w/h of zones on y

		//           o-----------------o       
		//           |                 |
		//          top----------------o
		// 
		//   o---o   o---------------ssize    o---o
		//   |   |   |                 |      |   |
		//   |   |   |                 |      |   |
		//   |   |   |                 |      |   |
		//   |   |   |                 |      |   |
		// left--o -ssize--------------o   right--o
		//           
		//           o-----------------o
		//           |                 |
		//          bot----------------o

		vec2 top   = vec2(-screenSize.x,                           screenSize.y + padding);
		vec2 bot   = vec2(-screenSize.x,                          -screenSize.y - padding - m_zoneSize.x);
		vec2 left  = vec2(-screenSize.x - padding - m_zoneSize.y, -screenSize.y);
		vec2 right = vec2( screenSize.x + padding,                -screenSize.y);

		for (int i = 0; i < numberOfZones; i++)
		{
			vec2 minTop   = top   + (float)i * vec2(m_zoneSize.x, 0);
			vec2 minBot   = bot   + (float)i * vec2(m_zoneSize.x, 0);
			vec2 minLeft  = left  + (float)i * vec2(0, m_zoneSize.y);
			vec2 minRight = right + (float)i * vec2(0, m_zoneSize.y);
			
			m_zones.push_back(SpawningZone{ minTop });
			m_zones.push_back(SpawningZone{ minBot });
			m_zones.push_back(SpawningZone{ minLeft });
			m_zones.push_back(SpawningZone{ minRight });
		}
	}

	float delay = 2.f;
	float time  = 2.f;

	void Update()
	{
		time -= Time::DeltaTime();
		if (time < 0)
		{
			time = delay;
			SpawnRandomEnemy(m_zones.at(get_rand((int)m_zones.size() - 1)));
		}
	}


private:

	void SpawnRandomEnemy(const SpawningZone& zone)
	{
		Entity entity;
		
		switch (get_rand(2))
		{
			case 0: // fighter
			{
				entity = CreateSandSprite("enemy_fighter.png", "enemy_fighter_mask.png");
				entity.Add<FireWeaponAfterDelay>(FirstEntityWith<Player>(), WEAPON_LASER, 1.f + get_randc(.3f));
				entity.Add<TurnTwoardsTarget>(CreateEntity().AddAll(Transform2D(get_randc(32), get_randc(18))));
				entity.Add<Flocker>();

				break;
			}

			case 1: // bomb
			{
				entity = CreateSandSprite("enemy_bomb.png", "enemy_bomb_mask.png");
				entity.Add<TurnTwoardsTarget>(FirstEntityWith<Player>());

				break;
			}

			case 2: // station
			{
				entity = CreateSandSprite("enemy_station.png", "enemy_station_mask.png");
				entity.Add<TurnTwoardsTarget>(CreateEntity().AddAll(Transform2D(get_rand(32, 18))));
				entity.Add<Flocker>();

				break;
			}
		}

		Transform2D& transform = entity.Get<Transform2D>();
		transform.position = zone.min + vec2(get_rand(m_zoneSize.x), get_rand(m_zoneSize.y));

		entity.Add<Rigidbody2D>(transform).SetFixedRotation(true).SetVelocity(get_randn(5.f));
	}
};