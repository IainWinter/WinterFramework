#pragma once

#include "Leveling.h"
#include "Events.h"
#include "Physics.h"

#include "Components/Lightning.h"

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
		vec2 direction = normalize(target.Get<Transform2D>().position - position) + get_randn(inaccuracy);
		
		position += direction * .5f;

		vec2 normal = vec2(direction.y, -direction.x) * get_randc(2.f);

		Entity e = CreateEntity();
		Transform2D& transform = e.Add<Transform2D>(position);

		switch (weapon)
		{
			case WEAPON_CANNON:
			{
				e.Add<CellProjectile>(entity.Id());
				e.Add<DestroyInTime>(3.f);
				e.Add<ParticleEmitter>(GetPrefab_BulletEmitter());
				
				transform.position += normal * .05f;

				GetModule<PhysicsWorld>().AddEntity(e)
					.SetVelocity(direction * 100.f);

				break;
			}
			
			case WEAPON_MINIGUN:
			{
				e.Add<CellProjectile>(entity.Id());
				e.Add<DestroyInTime>(5.f);
				e.Add<ParticleEmitter>(GetPrefab_BulletEmitter());
				
				transform.position += normal * .07f;

				GetModule<PhysicsWorld>().AddEntity(e)
					.SetVelocity(direction * 40.f);

				break;
			}

			case WEAPON_LASER: 
			{
				e.Add<CellProjectile>(entity.Id());
				e.Add<DestroyInTime>(5.f);
				e.Add<ParticleEmitter>(GetPrefab_LaserEmitter());

				transform.position += normal * .02f;
				
				GetModule<PhysicsWorld>().AddEntity(e)
					.SetVelocity(direction * 25.f);

				break;
			}

			case WEAPON_LASER_LARGE:
			{
				e.Add<CellProjectile>(entity.Id());
				e.Add<DestroyInTime>(5.f);
				e.Add<ParticleEmitter>(GetPrefab_LaserEmitter());

				transform.position += normal * .5f;

				GetModule<PhysicsWorld>().AddEntity(e)
					.SetVelocity(direction * 60.f);

				break;
			}

			case WEAPON_FUEL_SHOT:
			{
				e.Add<CellProjectile>(entity.Id());
				e.Add<DestroyInTime>(3.f);
				e.Add<ParticleEmitter>(GetPrefab_FuelShotEmitter());

				GetModule<PhysicsWorld>().AddEntity(e)
					.SetVelocity(direction * 50.f);

				break;
			}

			case WEAPON_BOLTZ:
			{
				e.Add<ParticleEmitter>(GetPrefab_LightningEmitter()).enableAutoEmit = false;
				e.Add<Lightning>(entity, target);
			}
		}
	}
};