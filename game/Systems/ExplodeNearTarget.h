#pragma once

#include "Leveling.h"
#include "Events.h"
#include "ext/Time.h"
#include "Components/EnemyAI.h"

struct System_ExplodeNearTarget : SystemBase
{
	void Update()
	{
		for (auto [transform, bomb] : Query<Transform2D, ExplodeNearTarget>())
		{
			if (!bomb.m_exploded && bomb.fuse <= 0.f)
			{
				bomb.m_exploded = true;
				Send(event_SpawnExplosion { transform.position, bomb.explosionPower });
			}

			else if (bomb.m_tickFuse)
			{
				bomb.fuse -= Time::DeltaTime();
			}

			else if (IsNearTarget(transform, bomb))
			{
				bomb.m_tickFuse = true;
			}
		}
	}

private:

	bool IsNearTarget(const Transform2D& transform, const ExplodeNearTarget& bomb)
	{
		return distance(transform.position, bomb.target.Get<Transform2D>().position) < bomb.distanceToStartFuse;
	}
};