#pragma once

#include "Leveling.h"
#include "Components/EnemyAI.h"

struct System_EnemyController : System<System_EnemyController>
{
	void Update()
	{
		//for (auto [entity, bomb] : QueryWithEntity<ExplodeNearTarget>())
		//{
		//	if (bomb.m_tickFuse)
		//	{
		//		if (entity.Has<TurnTwoardsTarget>()) entity.Remove<TurnTwoardsTarget>();
		//		if (entity.Has<Flocker>())           entity.Remove<Flocker>();
		//	}
		//}

		for (auto [body, flocker] : Query<Rigidbody2D, Flocker>())
		{
			body.SetVelocity(normalize(body.GetVelocity()) * 10.f); // set constant velocity for flockers
		}
	}
};