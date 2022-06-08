#pragma once

#include "Leveling.h"
#include "Player.h"
#include "CoordTranslation.h"

struct PlayerController : System<PlayerController>
{
	Entity playerEntity;

	void Init()
	{
		Attach<event_Input>();
		playerEntity = FirstEntityWith<Player>();
	}

	void Update()
	{
		auto [player, body] = playerEntity.GetAll<Player, Rigidbody2D>();

		vec2 vel = lerp(
			body.GetVelocity(),
			safe_normalize(player.MovementInput) * player.MovementSpeed,
			Time::DeltaTime() * player.MovementAccelerationScaleFactor);

		body.SetVelocity(vel);
	}

	void on(event_Input& e)
	{
		Player& player = playerEntity.Get<Player>();

		if (e.name == InputName::UP)    player.MovementInput.y += e.state;
		if (e.name == InputName::DOWN)  player.MovementInput.y -= e.state;
		if (e.name == InputName::RIGHT) player.MovementInput.x += e.state;
		if (e.name == InputName::LEFT)  player.MovementInput.x -= e.state;
	}

	// translates mouse to controller

	void on(event_Mouse& e)
	{
		Transform2D& t = playerEntity.Get<Transform2D>();

		// calculate the direction from the player to the target
		vec2 target = vec2(e.screen_x, e.screen_y) * GetModule<CoordTranslation>().ScreenToWorld;
		vec2 relitiveTarget = target - vec2(t.x, t.y);

		SendNow(event_Input{ InputName::AIM_X, relitiveTarget.x });
		SendNow(event_Input{ InputName::AIM_Y, relitiveTarget.y });
	}
};