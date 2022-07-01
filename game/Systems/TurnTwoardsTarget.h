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
			//body.ApplyForce(body.GetMass() * CalcForceTwoardsTarget(body, turn));
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

		return safe_normalize(body.GetVelocity() + delta) * speed;// *clamp(length(targetVel), speed * turn.atTargetVelocityDampen, speed);
	}

	vec2 CalcForceTwoardsTarget(const Rigidbody2D& body, const TurnTwoardsTarget& turn)
	{
		vec2 dir = turn.target.Get<Transform2D>().position - body.GetPosition();
		float speed = length(body.GetVelocity());
		float weight = clamp(length(dir) / speed * 100.f, 15.f * turn.atTargetVelocityDampen, 15.f);
		
		// if speed is too fast, slow down
		// doesnt really work
		// if (speed > 20 && dot(safe_normalize(dir), safe_normalize(body.GetVelocity())) > .9) weight = -weight;
		//printf("%f %f\n", speed, weight);

		return safe_normalize(dir) * weight;
	}
};