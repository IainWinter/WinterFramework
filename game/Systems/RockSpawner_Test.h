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
	float spawnTime = 6.f;

	float rockSpeed = 10.f;

	void Update()
	{
		spawnTimer -= Time::DeltaTime();
		if (spawnTimer < 0.f)
		{
			spawnTimer = spawnTime;
			
			vec2 pos = get_randnc(40.f);
			vec2 vel = -normalize(pos) * rockSpeed;
			r<Texture> texture = mkr<Texture>(_a("asteroid_mid_1.png"), false);

			Entity rock = CreateEntity();
			rock.Add<Transform2D>(pos);
			rock.Add<Sprite>(texture);
			rock.Add<SandSprite>(texture);

			rock.Add<Rigidbody2D>()
				.SetPosition(pos)
				.SetVelocity(vel)
				.SetAngularVelocity(get_randc(w2PI/ 7.f))
				.SetDensity(1000.f);

			Send(event_SandAddSprite{ rock });
		}
	}
};