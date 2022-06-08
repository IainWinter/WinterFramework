#pragma once

#include "Leveling.h"
#include "Physics.h"
#include "Flocker.h"

struct FlockingMovement : SystemBase
{
	void Update()
	{
		for (auto [body, flocker] : Query<Rigidbody2D, Flocker>())
		{
			// Arbitrarily weight these forces
            auto [sep, ali, coh] = CalcForces(body, flocker);
			body.ApplyForce(sep * 1.5f + ali * 1.f + coh * 1.f);
		}
	}

private:

    // credit Daniel Shiffman (https://processing.org/examples/flocking.html)

	std::tuple<vec2, vec2, vec2> CalcForces(const Rigidbody2D& body, const Flocker& flocker)
	{
        vec2 forceSeperation = vec2(0.f, 0.f);
        vec2 forceAlign      = vec2(0.f, 0.f);
        vec2 forceCohesion   = vec2(0.f, 0.f);

        int numberSeperation = 0;
        int numberClose      = 0;

        for (auto [bodyOther, flockerOther] : Query<Rigidbody2D, Flocker>())
        {
            vec2 pos = body.GetPosition();
            vec2 other = bodyOther.GetPosition();
            float d = distance(pos, other);

            if (d < 0.001f) continue; // guard against self / boid too close

            if (d < flocker.desiredSeparation)
            {
                forceSeperation  += normalize(pos - other) / d;
                numberSeperation += 1;
            }

            if (d < flocker.neighborDist)
            {
                forceAlign    += bodyOther.GetVelocity();
                forceCohesion += bodyOther.GetPosition();
                numberClose   += 1;
            }
        }

        forceSeperation /= (float)clamp(numberSeperation, 1, INT_MAX);
        forceAlign      /= (float)clamp(numberClose,      1, INT_MAX);
        forceCohesion   -= body.GetPosition();

        forceSeperation = clamp(safe_normalize(forceSeperation) * flocker.maxSpeed - body.GetVelocity(), flocker.maxForceFactor);
        forceAlign      = clamp(safe_normalize(forceAlign)      * flocker.maxSpeed - body.GetVelocity(), flocker.maxForceFactor);
        forceCohesion   = clamp(safe_normalize(forceCohesion)   * flocker.maxSpeed - body.GetVelocity(), flocker.maxForceFactor);

        return { forceSeperation, forceAlign, forceCohesion };
	}
};