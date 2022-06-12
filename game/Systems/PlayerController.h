#pragma once

#include "Leveling.h"
#include "Player.h"
#include "CoordTranslation.h"
#include "Windowing.h"
#include "ext/Sand.h"

#include "Events.h"

struct System_PlayerController : System<System_PlayerController>
{
	Entity playerEntity;

	void Init()
	{
		Attach<event_Input>();
		Attach<event_Mouse>();
		playerEntity = FirstEntityWith<Player>();
	}

	void Update()
	{
		auto [player, transform] = playerEntity.GetAll<Player, Transform2D>();

		vec2 direction = safe_normalize(player.AttackDirectionInput);
		vec2 position = transform.position + direction * 1.f;
		
		direction *= 600.f;

		if (player.AttackFireInput)
		{
			GetModule<SandWorld>()
				.CreateCell(position.x, position.y, Color(255, 0, 0, 255), direction.x /*+ get_randc(100.f)*/, direction.y /*+ get_randc(100.f)*/, 1.f)
				.AddAll(CellLife{ 5.f }, CellProjectile{ playerEntity.Id() });
		}

		//if (player.AttackFireInput)
		//{
		//	Send(event_SpawnExplosion { vec2(20, 0), 20 });
		//}
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

			case InputName::AIM_X:  player.AttackDirectionInput.x = e.state;        break;
			case InputName::AIM_Y:  player.AttackDirectionInput.y = e.state;        break;
			case InputName::ATTACK: player.AttackFireInput        = e.state != 0.f; break;
		}
	}

	// translates mouse to controller

	void on(event_Mouse& e)
	{
		Transform2D& transform = playerEntity.Get<Transform2D>();

		// calculate the direction from the player to the target
		vec2 target = vec2(e.screen_x, e.screen_y) * GetModule<CoordTranslation>().ScreenToWorld;
		vec2 relitiveTarget = target - transform.position;

		SendNow(event_Input{ InputName::AIM_X, relitiveTarget.x });
		SendNow(event_Input{ InputName::AIM_Y, relitiveTarget.y });
		SendNow(event_Input{ InputName::ATTACK, (float)e.button_left });
	}
};
