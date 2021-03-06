#pragma once

#include "app/System.h"
#include "Physics.h"
#include "Components/Flocker.h"

struct System_FlockingMovement : SystemBase
{
	void FixedUpdate()
	{
		for (auto [body, flocker] : Query<Rigidbody2D, Flocker>())
		{
			body.ApplyForce(body.GetMass() * CalcForces(body, flocker));

            if (body.GetSpeed() > flocker.maxSpeed)
            {
                body.SetVelocity(limit(body.GetVelocity(), flocker.maxSpeed));
            }
		}
	}

private:

    // credit Daniel Shiffman (https://processing.org/examples/flocking.html) <3

	vec2 CalcForces(const Rigidbody2D& body, const Flocker& flocker)
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

            if (d < flocker.desiredSeparation) // might want to add a bounding radius, then desired seperation would be the dist between the objs not centers
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

        forceSeperation = limit(safe_normalize(forceSeperation) * flocker.maxSpeed - body.GetVelocity(), flocker.maxForceFactor);
        forceAlign      = limit(safe_normalize(forceAlign)      * flocker.maxSpeed - body.GetVelocity(), flocker.maxForceFactor);
        forceCohesion   = limit(safe_normalize(forceCohesion)   * flocker.maxSpeed - body.GetVelocity(), flocker.maxForceFactor);

        // Arbitrarily weight these forces

        return 1.5f * forceSeperation + 1.f * forceAlign + 1.f * forceCohesion;
	}
};