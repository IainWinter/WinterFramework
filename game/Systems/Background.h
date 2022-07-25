#pragma once

#include "app/System.h"
#include "Prefabs.h"

struct SmokeComponent { int _pad; };
struct  StarComponent { int _pad; };

struct Background_System : System<Background_System>
{
	void Init()
	{
		CreateEntity()
			.Add<Camera>(0, 0, 32, 18);

		for (int i = 0; i < 10; i++)
		{
			Entity smoke = CreateEntity();
		
			smoke.Add<Transform2D>()
				.SetPosition(get_randc(50.f, 50.f))
				.SetScale   (vec2(10.f + get_rand(20.f)))
				.SetZIndex  (-5.f);

			smoke.Add<Sprite>()
				.SetSource(GetPrefab_Texture("smoke.png"))
				.SetTint(Color(100, get_rand(200), 200, 180));

			smoke.Add<Rigidbody2D>()
				.SetVelocity(get_randc(2.f, 2.f));

			smoke.Add<SmokeComponent>();
		}

		for (int i = 0; i < 1000; i++)
		{
			Entity star = CreateEntity();

			star.Add<Transform2D>()
				.SetPosition(get_randc(32.f, 18.f) * 2.f)
				.SetScale   (vec2(get_rand(.05f)))
				.SetZIndex  (-9.f);

			star.Add<Sprite>()
				.SetSource(GetPrefab_Texture("diamond.png"));

			star.Add<StarComponent>();
		}
	}

	void Update()
	{
		for (auto [smoke, body] : Query<SmokeComponent, Rigidbody2D>())
		{
			vec2 vel = body.GetVelocity();
			vel += -safe_normalize(body.GetPosition()) / 1000.f; // orbit center to keep dust on screen
			body.SetVelocity(vel);
		}

		for (auto [star, transform, sprite] : Query<StarComponent, Transform2D, Sprite>())
		{
			transform.rotation += Time::DeltaTime() / 100.f;

			sprite.SetTint( Color(128 + get_rand(127), 255, 255, 255) );
		}
	}
};