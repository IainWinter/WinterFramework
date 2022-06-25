#pragma once

#include "Leveling.h"
#include "Sand/SandEvents.h"

#include "ext/rendering/Particle.h"

struct Sand_System_CollideProjectile : System<Sand_System_CollideProjectile>
{
	Particle smoke;

	void Init()
	{
		smoke = Particle(
			mkr<TextureAtlas>(mkr<Texture>(_a("fire_burst.png")), 9, 9), 75
		);

		//smoke.repeatCount = 100;

		Attach<event_Sand_ProjectileHit>();
	}

	void on(event_Sand_ProjectileHit& e)
	{
		auto [sprite, mask] = e.entity.GetAll<Sprite, SandSprite>();
		int index = sprite.Get().Index32(e.hitPosInSprite.x, e.hitPosInSprite.y);
		
		// should make this cell slow down over time...

		if (sprite.Get().At(index).a > 0)
		{
			Cell cell = e.projectile.Get<Cell>(); 
			cell.vel = cell.vel / (get_rand(5.f) + 1.f);
			cell.vel += get_randn(length(cell.vel) / 4.f);
			cell.life = .2f;
			cell.color = sprite.Get().At(index);
	
			Send(event_Sand_CreateCell(cell));

			printf("spawnned particle %f\n", Time::TotalTime());

			Entity entity = CreateEntity().AddAll(
				Transform2D(vec3(e.hitPosInWorld, 2.f), vec2(1.5f, 1.5f), get_rand(2.f * pi<float>())),
				Particle(smoke)
			);

			GetModule<PhysicsWorld>().AddEntity(entity).SetVelocity(get_randn(2.f));
		}
		
		sprite.Get().At(index).a = 0;
		mask  .Get().At(index).a = 0;
		mask.cellCount -= 1;
	}
};