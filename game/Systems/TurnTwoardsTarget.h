#pragma once

#include "Leveling.h"
#include "Physics.h"
#include "Components/EnemyAI.h"

struct System_TurnTwoardsTarget : SystemBase
{
	void FixedUpdate()
	{
		for (auto [body, turn] : Query<Rigidbody2D, TurnTwoardsTarget>())
		{
			body.SetVelocity(CalcTurnedVelocity(body, turn));
		}
	}

private:

	vec2 CalcTurnedVelocity(const Rigidbody2D& body, const TurnTwoardsTarget& turn)
	{
		vec2 targetVel = turn.target.Get<Transform2D>().position - body.GetPosition();
		float speed = length(body.GetVelocity());

		vec2 nVel = safe_normalize(body.GetVelocity());
		vec2 nDir = safe_normalize(targetVel);
		vec2 delta = (nDir - nVel) * turn.strength;

		return safe_normalize(body.GetVelocity() + delta) * speed;
	}
};