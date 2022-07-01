#pragma once

#include "Leveling.h"
#include "CoordTranslation.h"
#include "Windowing.h"
#include "Sand/Sand.h"
#include "Events.h"
#include "ext/rendering/Particle.h"
#include "Components/Player.h"
#include "Components/EnemyAI.h"
#include "Prefabs.h"

struct System_PlayerController : System<System_PlayerController>
{
	Entity playerEntity;
	Entity target;

	void Init()
	{
		Attach<event_Input>();
		Attach<event_Mouse>();
		playerEntity = FirstEntityWith<Player>();
		target = CreateEntity().AddAll(Transform2D(vec2(0.f, 0.f)));
	}

	void Update()
	{
		Player& player = playerEntity.Get<Player>();
		
		player.m_attackTimer -= Time::DeltaTime();
		if (player.AttackFireInput && player.m_attackTimer <= 0.f)
		{
			player.m_attackTimer = player.AttackTime;
			Send(event_FireWeapon{playerEntity, target, player.CurrentWeapon});
		}

		target.Get<Transform2D>().position = player.AttackLocationInput;
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
