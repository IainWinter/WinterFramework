#pragma once

#include "app/System.h"
#include "Events.h"
#include "Physics.h"

#include "Components/Lightning.h"

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
			log_game("Warning: Target is dead.");
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
				e.Add<DestroyInTime>(2.f);
				//e.Add<ParticleEmitter>(GetPrefab_BulletEmitter());
				
				e.Add<Sprite>(GetPrefab_Texture("diamond.png"));

				e.Add<CellProjectile>(entity.Id())
					.SetHealth(50)
					.SetSize(1)
					.SetTurnRate(0.5f);

				e.Add<WrapOnScreen>();

				PlaySound("event:/fire_main_gun");

				transform.position += normal * .05f;
				transform.scale = vec2(First<CoordTranslation>().CellsToMeters * 4);

				e.Add<Rigidbody2D>()
					.SetVelocity(direction * 24.f);

				break;
			}
			
			case WEAPON_MINIGUN:
			{
				e.Add<DestroyInTime>(5.f);
				e.Add<ParticleEmitter>(GetPrefab_BulletEmitter());
				
				transform.position += normal * .07f;

				e.Add<CellProjectile>(entity.Id())
					.SetHealth(5);

				e.Add<Rigidbody2D>()
					.SetVelocity(direction * 40.f);

				break;
			}

			case WEAPON_LASER: 
			{
				e.Add<DestroyInTime>(5.f);
				e.Add<ParticleEmitter>(GetPrefab_LaserEmitter());

				e.Add<CellProjectile>(entity.Id())
					.SetHealth(50);

				transform.position += normal * .02f;
				
				e.Add<Rigidbody2D>()
					.SetVelocity(direction * 25.f);

				break;
			}

			case WEAPON_LASER_LARGE:
			{
				e.Add<DestroyInTime>(5.f);
				e.Add<ParticleEmitter>(GetPrefab_LaserEmitter());

				e.Add<CellProjectile>(entity.Id())
					.SetHealth(50);

				transform.position += normal * .5f;

				e.Add<Rigidbody2D>()
					.SetVelocity(direction * 60.f);

				break;
			}

			case WEAPON_FUEL_SHOT:
			{
				e.Add<DestroyInTime>(3.f);
				e.Add<ParticleEmitter>(GetPrefab_FuelShotEmitter());

				e.Add<CellProjectile>(entity.Id())
					.SetHealth(15)
					.SetSize(3);

				e.Add<Rigidbody2D>()
					.SetVelocity(direction * 50.f);

				break;
			}

			case WEAPON_BOLTZ:
			{
				e.Add<ParticleEmitter>(GetPrefab_LightningEmitter()).enableAutoEmit = false;
				e.Add<Lightning>(entity, target);
				e.Add<LightningDamage>(5);
			}
		}
	}
};