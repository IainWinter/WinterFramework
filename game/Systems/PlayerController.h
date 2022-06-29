#pragma once

#include "Leveling.h"
#include "CoordTranslation.h"
#include "Windowing.h"
#include "Sand/Sand.h"
#include "Events.h"
#include "ext/rendering/Particle.h"
#include "Components/Player.h"
#include "Components/EnemyAI.h"

struct System_PlayerController : System<System_PlayerController>
{
	Entity playerEntity;
	Entity fireballEntity;
	//Entity sandTestParticleEntity;

	Entity target;

	void Init()
	{
		Attach<event_Input>();
		Attach<event_Mouse>();
		playerEntity = FirstEntityWith<Player>();

		Particle fireball = Particle(mkr<TextureAtlas>(mkr<Texture>(_a("diamond.png"))));
		fireball.repeatCount = 10;
		fireball.orignal.scale = vec2(1, .5f);
		fireball.tints = {
			Color( 10, 147, 255,   0),
			Color(244,  86,  12, 153),
			Color(255,  28,   0, 184)
		};

		fireballEntity = CreateEntity();
		fireballEntity.Add<Transform2D>(vec3(20.f, 0, 4.f));
		fireballEntity.Add<ParticleEmitter>().AddSpawner(fireball, 1.f,
			[this](Particle particle)
			{
				Entity entity = CreateEntity();
				entity.Add<Particle>(particle);
				entity.Add<Transform2D>(particle.orignal);

				entity.Add<ParticleShrinkWithAge>();

				Rigidbody2D&  body = GetModule<PhysicsWorld>().AddEntity(entity);
				body.SetAngularVelocity(get_randc(10.f));
				body.SetAngle(get_rand(w2PI));
				body.SetVelocity(get_randn(5.f));
				body.SetDamping(5.f);
			});


		//SandWorld& sand = GetModule<SandWorld>();

		//Particle sqr = Particle(mkr<TextureAtlas>(mkr<Texture>(_a("sqr_small.png"))));
		//sqr.framesPerSecond = 100;
		//sqr.orignal.scale = vec2(1.f / sand.cellsPerMeter);

		//sandTestParticleEntity = CreateEntity();
		//sandTestParticleEntity.Add<ParticleEmitter>().AddSpawner(sqr, 1.f, 
		//	[this](Particle particle)
		//	{
		//		Entity entity = CreateEntity();
		//		entity.Add<Particle>(particle);
		//		entity.Add<Transform2D>(particle.orignal);
		//	});


		target = CreateEntity().AddAll(Transform2D(vec2(10.f, 0.f)));
	}

	void Update()
	{
		auto [player, transform, body] = playerEntity.GetAll<Player, Transform2D, Rigidbody2D>();

		vec2 direction = safe_normalize(player.AttackLocationInput - transform.position);
		vec2 position = transform.position + direction * 1.f;
		
		direction *= 100.f;

		player.m_attackTimer -= Time::DeltaTime();
		if (player.AttackFireInput && player.m_attackTimer <= 0.f)
		{
			player.m_attackTimer = player.AttackTime;
			Send(event_Sand_CreateCell(position, direction, Color(255, 255, 255), 5.f, [this](Entity e) 
			{ 
				e.Add<CellProjectile>(playerEntity.Id());
				e.Add<ParticleEmitter>(fireballEntity.Get<ParticleEmitter>());
			}));
		
			//Send(event_SpawnExplosion{vec2(10, 0), 20.f});
		}

		//sandTestParticleEntity.Get<Transform2D>().position = player.AttackLocationInput;
	}

	void FixedUpdate()
	{
		auto [player, body] = playerEntity.GetAll<Player, Rigidbody2D>();

		// allow for collision response...

		vec2 vel = lerp(
			body.GetVelocity(),
			safe_normalize(player.MovementInput) * player.MovementSpeed,
			Time::DeltaTime() * player.MovementAccelerationScaleFactor);

		body.SetVelocity(vel);
	}

	void on(event_Input& e)
	{
		Player& player = playerEntity.Get<Player>();

		switch (e.name)
		{
			case InputName::UP:    player.MovementInput.y += e.state; break;
			case InputName::DOWN:  player.MovementInput.y -= e.state; break;
			case InputName::RIGHT: player.MovementInput.x += e.state; break;
			case InputName::LEFT:  player.MovementInput.x -= e.state; break;

			case InputName::AIM_X:  player.AttackLocationInput.x = e.state;        break;
			case InputName::AIM_Y:  player.AttackLocationInput.y = e.state;        break;
			case InputName::ATTACK: player.AttackFireInput        = e.state != 0.f; break;
		}
	}

	// translates mouse to controller

	void on(event_Mouse& e)
	{
		Transform2D& transform = playerEntity.Get<Transform2D>();

		// calculate the direction from the player to the target
		vec2 target = vec2(e.screen_x, e.screen_y) * GetModule<CoordTranslation>().ScreenToWorld;
		//vec2 relitiveTarget = target - transform.position;

		SendNow(event_Input{ InputName::AIM_X, target.x });
		SendNow(event_Input{ InputName::AIM_Y, target.y });
		SendNow(event_Input{ InputName::ATTACK, (float)e.button_left });
	}
};
