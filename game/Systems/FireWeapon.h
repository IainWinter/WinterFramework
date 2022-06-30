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
		FireWeapon(e.owner, e.target, e.type);
	}

private:

	void FireWeapon(Entity entity, Entity target, Weapon weapon)
	{
		vec2 position = entity.Get<Transform2D>().position;
		vec2 direction = normalize(target.Get<Transform2D>().position - position);
		
		switch (weapon)
		{
			case LASER: 
			{
				Entity entity = CreateEntity();
				entity.Add<Transform2D>(position);
				entity.Add<DestroyInTime>(3.f);
				entity.Add<CellProjectile>(entity.Id());
				entity.Add<ParticleEmitter>(GetPrefab_LaserEmitter());
				GetModule<PhysicsWorld>().AddEntity(entity).SetVelocity(direction * 50.f);

				break;
			}

			case FUEL_SHOT:
			{
				Entity entity = CreateEntity();
				entity.Add<Transform2D>(position);
				entity.Add<DestroyInTime>(3.f);
				entity.Add<CellProjectile>(entity.Id());
				entity.Add<ParticleEmitter>(GetPrefab_FuelShotEmitter());
				GetModule<PhysicsWorld>().AddEntity(entity).SetVelocity(direction * 50.f);

				break;
			}
		}
	}
};