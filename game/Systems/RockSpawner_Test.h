#pragma once

#include "Leveling.h"
#include "Physics.h"
#include "ext/Time.h"
#include "ext/rendering/Sprite.h"
#include "Sand/SandEvents.h"
#include "Prefabs.h"

struct System_RockSpawner_Test : SystemBase
{
	float spawnTimer = 0.f;
	float spawnTime = 2.f;

	float rockSpeed = 10.f;

	void Update()
	{
		spawnTimer -= Time::DeltaTime();
		if (spawnTimer < 0.f)
		{
			spawnTimer = spawnTime;
			
			vec2 pos = get_randnc(50.f);
			vec2 vel = -normalize(pos) * rockSpeed;
			r<Texture> texture = GetPrefab_Texture("asteroid_mid_1.png", false);

			Entity rock = CreateEntity();
			rock.Add<Transform2D>(pos);
			rock.Add<Rigidbody2D>().SetPosition(pos).SetVelocity(vel);
			rock.Add<Sprite>(texture);
			rock.Add<SandSprite>(texture);

			Send(event_SandAddSprite{ rock });
		}
	}
};