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

		if (e.entity.Id() == e.projectile.Get<CellProjectile>().owner) // exit if owner
		{
			return;
		}

		int index = sprite.Get().Index32(e.hitPosInSprite.x, e.hitPosInSprite.y);
		
		// should make this cell slow down over time...

		if (sprite.Get().At(index).a > 0)
		{
			//vec2 pos = e.projectile.Get<Transform2D>().position;
			//vec2 vel = e.projectile.Get<Cell>().vel;
			//Color color = sprite.Get().At(index);
			//float life = .2f;
			//vel /= (get_rand(5.f) + 1.f) + get_randn(length(vel) / 4.f);
			//Send(event_Sand_CreateCell(pos, vel, color, life));

			//Entity entity = CreateEntity().AddAll(
			//	Transform2D(vec3(e.hitPosInWorld, 2.f), vec2(1.5f, 1.5f), get_rand(2.f * pi<float>())),
			//	Particle(smoke)
			//);

			//GetModule<PhysicsWorld>().AddEntity(entity).SetVelocity(get_randn(2.f));
		}

		Send(event_Sand_RemoveCell{ e.entity, index } );
	}
};