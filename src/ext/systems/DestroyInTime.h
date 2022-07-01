#pragma once

#include "Leveling.h"
#include "ext/Components.h"

struct System_DestroyInTime : SystemBase
{
	void Update()
	{
		for (auto [entity, destroyWithTime] : QueryWithEntity<DestroyInTime>())
		{
			destroyWithTime.InSeconds -= Time::DeltaTime();
			if (destroyWithTime.InSeconds <= 0.f)
			{
				entity.Destroy();
			}
		}
	}
};