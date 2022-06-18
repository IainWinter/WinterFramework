#pragma once

#include "Leveling.h"
#include "Sand/SandEvents.h"

struct Sand_System_CollideProjectile : System<Sand_System_CollideProjectile>
{
	void Init()
	{
		Attach<event_Sand_ProjectileHit>();
	}

	void on(event_Sand_ProjectileHit& e)
	{
		auto [sprite, mask] = e.entity.GetAll<Sprite, SandSprite>();

		int index = sprite.Get().Index32(e.hitPosInSprite.x, e.hitPosInSprite.y);
		
		sprite.Get().At(index).a = 0;
		mask  .Get().At(index).a = 0;
		mask.cellCount -= 1;

		Cell cell = e.projectile.Get<Cell>();
		cell.vel = cell.vel / (get_rand(5.f) + 1.f);
		cell.vel += get_randn(length(cell.vel) / 4.f);
		cell.life = .2f;
		cell.color = sprite.Get().At(index);
		Send(event_Sand_CreateCell(cell));
	}
};