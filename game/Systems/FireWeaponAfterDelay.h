#pragma once

#include "Leveling.h"
#include "ext/Time.h"

#include "Components/EnemyAI.h"
#include "Sand/SandEvents.h"

struct System_FireWeaponAfterDelay : SystemBase
{
	void Update()
	{
		for (auto [entity, fire] : QueryWithEntity<FireWeaponAfterDelay>())
		{
			fire.m_timer -= Time::DeltaTime();
			if (fire.m_timer <= 0.f)
			{
				fire.m_timer = fire.delay;
				FireWeapon(entity, fire.target, fire.weapon);
			}
		}
	}

private:

	void FireWeapon(Entity entity, Entity target, Weapon weapon)
	{
		vec2 position = entity.Get<Transform2D>().position;
		vec2 direction = normalize(target.Get<Transform2D>().position - position);
		switch (weapon)
		{
			case LASER: 
			{
				vec2 velocity = direction * 1000.f;
				Send(event_Sand_CreateCell(position, velocity, Color(255, 250, 250), 5.f, [entity](Entity e) { e.Add<CellProjectile>(entity.Id()); }));
				break;
			}
		}
	}
};