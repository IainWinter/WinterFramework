#pragma once

#include "app/System.h"
#include "Physics.h"
#include "Prefabs.h"
#include "Sand/SandEvents.h"
#include "ext/Time.h"
#include "ext/rendering/Sprite.h"
#include "util/random.h"

struct Asteroid {
	int _pad;
};

struct System_RockSpawner_Test : SystemBase
{
	float spawnTimer = 0.f;
	float spawnTime = 0.f;// 6.f;

	float rockSpeed = 10.f;
	float rockTurning = wPI / 2.f;

	int asteroidsLeft = 6;

	void Update()
	{
		spawnTimer -= Time::DeltaTime();
		if (spawnTimer < 0.f && asteroidsLeft > 0)
		{
			spawnTimer = spawnTime;
			asteroidsLeft -= 1;
			
			vec2 pos = get_randnc(40.f);
			vec2 vel = -normalize(pos) * rockSpeed * (.1f + get_rand(.9f));
			float avel = get_randc(1.f) * rockTurning;

			std::string asterName = choose<const char*>({
				{"asteroid_mid_1.png", 10},
				{"asteroid_mid_2.png", 20},
				{"asteroid_mid_3.png", 30},
				{"asteroid_mid_4.png", 40},
				{"asteroid_mid_5.png", 50}
			});

			r<Texture> texture = mkr<Texture>(_a(asterName), false);

			Entity rock = CreateEntity();
			rock.Add<Transform2D>(pos);
			rock.Add<Sprite>(texture);
			rock.Add<SandSprite>(texture)
				.SetCellStrength(.2f)
				.SetDensity(1000.f);

			rock.Add<Rigidbody2D>()
				.SetPosition(pos)
				.SetVelocity(vel)
				.SetAngularVelocity(avel)
				.SetDensity(1000.f);

			Send(event_SandAddSprite{ rock });
		}
	}
};