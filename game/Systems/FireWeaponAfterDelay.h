#pragma once

#include "app/System.h"
#include "ext/Time.h"

#include "Components/EnemyAI.h"
#include "Sand/SandEvents.h"
#include "Events.h"

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
				Send(event_FireWeapon{ entity, fire.target, fire.weapon, fire.inaccuracy });
			}
		}
	}
};