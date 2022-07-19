#pragma once

#include "app/System.h"
#include "Sand/SandHelpers.h"
#include "ext/rendering/Sprite.h"
#include "ext/Time.h"

struct System_LowCorePixelDeath : SystemBase
{
	void Update()
	{
		for (auto [entity, transform, sprite, ssprite, dieInTime] : QueryWithEntity<Transform2D, Sprite, SandSprite, SandDieInTimeWithLowCoreCount>())
		{
			if (dieInTime.m_timeToDie < 0.f)
			{
				Send(event_Sand_ExplodeToDust{ entity });
			}

			if (dieInTime.m_runTimer)
			{
				dieInTime.m_timeToDie -= Time::DeltaTime();
				sprite.tint = lerp(Color(255, 255, 255), Color(255, 79, 79), 1 - dieInTime.m_timeToDie / dieInTime.initalTimeToDie);
				transform.position += get_randn(dieInTime.jitterInMeters * (1 - dieInTime.m_timeToDie / dieInTime.initalTimeToDie));
			}

			else
			if (ssprite.pixels.CoreSize() < ssprite.initalCore.size() / 2)
			{
				dieInTime.m_runTimer = true;
				dieInTime.m_timeToDie = dieInTime.initalTimeToDie;
			}

			else
			{
				dieInTime.m_runTimer = false; // stop if healed past death amount
			}
		}
	}
};