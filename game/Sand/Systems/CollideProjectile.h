#pragma once

#include "Leveling.h"
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
		if (  !e.entity.IsAlive() 
			&& e.entity.Id() == e.projectile.Get<CellProjectile>().owner) // exit if owner or dead?
		{
			return;
		}

		auto [sprite, mask] = e.entity.GetAll<Sprite, SandSprite>();

		if (sprite.Get().At(e.index).a > 0) // projectiles hit mask sprite
		{
			Send(event_Sand_ExplodeToDust{ e.entity, {e.index}, sqrt(e.projectile.Get<Rigidbody2D>().GetVelocity()) });
		}

		Send(event_Sand_RemoveCell{ e.entity, e.index, e.hitPosInWorld });
	}
};