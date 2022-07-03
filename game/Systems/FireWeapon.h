#pragma once

#include "Leveling.h"
#include "Events.h"
#include "Physics.h"

#include "ext/Components.h"
#include "ext/rendering/Particle.h"
#include "Sand/SandComponents.h"

#include "Prefabs.h"

struct System_FireWeapon : System<System_FireWeapon>
{
	void Init()
	{
		Attach<event_FireWeapon>();
	}

	void on(event_FireWeapon& e)
	{
		FireWeapon(e.owner, e.target, e.type, e.inaccuracy);
	}

private:

	void FireWeapon(Entity entity, Entity target, Weapon weapon, float inaccuracy)
	{
		if (!target.IsAlive())
		{
			printf("Warning: Target is dead.\n");
			return;
		}

		vec2 position = entity.Get<Transform2D>().position;
		vec2 direction = normalize(target.Get<Transform2D>().position - position + get_randn(inaccuracy));
		
		//position += direction;

		Entity e = CreateEntity();
		e.Add<Transform2D>(position);
		e.Add<CellProjectile>(entity.Id());
		e.Add<DestroyInTime>(3.f);

		Rigidbody2D& body = GetModule<PhysicsWorld>().AddEntity(e);

		switch (weapon)
		{
			case WEAPON_CANNON: 
			{
				e.Add<ParticleEmitter>(GetPrefab_BulletEmitter());
				body.SetVelocity(direction * 30.f);

				break;
			}

			case WEAPON_LASER: 
			{
				e.Add<ParticleEmitter>(GetPrefab_LaserEmitter());
				body.SetVelocity(direction * 50.f);

				break;
			}

			case WEAPON_FUEL_SHOT:
			{
				e.Add<ParticleEmitter>(GetPrefab_FuelShotEmitter());
				body.SetVelocity(direction * 50.f);

				break;
			}
		}
	}
};