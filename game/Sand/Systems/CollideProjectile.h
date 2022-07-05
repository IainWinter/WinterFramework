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
		if (e.entity.Id() == e.projectile.Get<CellProjectile>().owner) // exit if owner
		{
			return;
		}

		auto [sprite, mask] = e.entity.GetAll<Sprite, SandSprite>();
		int index = sprite.Get().Index32(e.hitPosInSprite.x, e.hitPosInSprite.y);
		
		if (sprite.Get().At(index).a > 0) // projectiles hit mask sprite
		{
			Send(event_Sand_ExplodeToDust{ e.entity, {index}, e.projectile.Get<Rigidbody2D>().GetVelocity() / 3.f });
		}

		Send(event_Sand_RemoveCell{ e.entity, index } );
	}
};