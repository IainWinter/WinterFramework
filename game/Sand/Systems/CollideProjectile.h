#pragma once

#include "app/System.h"
#include "Sand/SandEvents.h"
#include "ext/rendering/Particle.h"

struct Sand_System_CollideProjectile : System<Sand_System_CollideProjectile>
{
	void Init()
	{
		Attach<event_Sand_ProjectileHit>();
	}

	void on(event_Sand_ProjectileHit& e)
	{
		if (   !e.entity.IsAlive()
			|| !e.projectile.IsAlive()
			||  e.entity.Id() == e.projectile.Get<CellProjectile>().owner) // exit if owner or dead?
		{
			return;
		}

		auto [sprite, mask, body] = e.entity.GetAll<Sprite, SandSprite, Rigidbody2D>();

		if (sprite.Get().At(e.index).a > 0) // projectiles hit mask sprite
		{
			Send(event_Sand_ExplodeToDust{ e.entity, {e.index}, sqrt(e.projectile.Get<Rigidbody2D>().GetVelocity()) });
		}

		Send(event_Sand_RemoveCell{ e.entity, e.index, e.hitPosInWorld });

		vec2 force = safe_normalize(e.projectile.Get<Rigidbody2D>().GetVelocity()) * 5.f;
		body.ApplyForce(force, e.hitPosInWorld - body.GetPosition());
	}
};