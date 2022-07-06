#pragma once

#include "Leveling.h"
#include "ext/Time.h"

struct System_Testing : SystemBase
{
	float t, delay = 2.f;

	Entity entity;
	Entity target;
	Weapon weapon = WEAPON_LASER;

	void Init()
	{
		entity = CreateEntity();
		target = CreateEntity();

		entity.Add<Transform2D>(vec2(-10, 10));
		target.Add<Transform2D>(vec2( 10, 10));
	}

	void Update()
	{
		t += Time::DeltaTime();

		if (t > delay)
		{
			t = 0.f;
			Send(event_FireWeapon{ entity, target, weapon });
		}
	}
};