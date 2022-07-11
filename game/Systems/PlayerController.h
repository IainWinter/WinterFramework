#pragma once

#include "Leveling.h"
#include "CoordTranslation.h"
#include "Windowing.h"
#include "Sand/Sand.h"
#include "Events.h"
#include "ext/rendering/Particle.h"
#include "Components/Player.h"
#include "Components/EnemyAI.h"
#include "Prefabs.h"

struct System_PlayerController : System<System_PlayerController>
{
	Entity playerEntity;
	Entity target;

	void Init()
	{
		Attach<event_Input>();
		Attach<event_Mouse>();
		Attach<event_Item_Pickup>();

		playerEntity = FirstEntityWith<Player>();
		target = CreateEntity().AddAll(Transform2D(vec2(0.f, 0.f)));
	}

	void Update()
	{
		if (!playerEntity.IsAlive()) return;

		Player& player = playerEntity.Get<Player>();
		player.m_attackTimer -= Time::DeltaTime();
		
		if (player.AttackFireInput && player.m_attackTimer <= 0.f)
		{
			player.m_attackTimer = player.CurrentWeaponAttackTime;
			Send(event_FireWeapon{ playerEntity, target, player.CurrentWeapon, player.CurrentWeaponInaccuracy });

			if (player.CurrentWeapon != WEAPON_CANNON)
			{
				player.CurrentWeaponAmmo -= 1;
				if (player.CurrentWeaponAmmo <= 0)
				{
					Send(event_Item_Pickup{ playerEntity, ITEM_WEAPON_CANNON });
				}
			}

			//CellCollisionInfo& info = GetModule<SandWorld>().GetCollisionInfo(player.AttackLocationInput);
			//if (info.hasHit)
			//{
			//	Entity e = Wrap(info.spriteEntityID);
			//	SandSprite& ssprite = e.Get<SandSprite>();
			//	Send(event_Sand_RemoveCell{ 
			//		Wrap(info.spriteEntityID), 
			//		ssprite.colliderMask->Index32(info.spriteHitIndex.x, info.spriteHitIndex.y), 
			//		player.AttackLocationInput
			//	});
			//}

			//Send(event_SpawnExplosion{player.AttackLocationInput, 20.f});
		}

		target.Get<Transform2D>().position = player.AttackLocationInput;
	}

	void FixedUpdate()
	{
		if (!playerEntity.IsAlive()) return;

		auto [player, body] = playerEntity.GetAll<Player, Rigidbody2D>();

		vec2 vel = lerp(
			body.GetVelocity(),
			safe_normalize(player.MovementInput) * player.MovementSpeed,
			Time::DeltaTime() * player.MovementAccelerationScaleFactor);

		body.SetVelocity(vel);
	}

	void on(event_Input& e)
	{
		if (!playerEntity.IsAlive()) return;

		Player& player = playerEntity.Get<Player>();
		switch (e.name)
		{
			case InputName::UP:     player.MovementInput.y += e.state; break;
			case InputName::DOWN:   player.MovementInput.y -= e.state; break;
			case InputName::RIGHT:  player.MovementInput.x += e.state; break;
			case InputName::LEFT:   player.MovementInput.x -= e.state; break;

			case InputName::AIM_X:  player.AttackLocationInput.x = e.state;        break;
			case InputName::AIM_Y:  player.AttackLocationInput.y = e.state;        break;
			case InputName::ATTACK: player.AttackFireInput       = e.state != 0.f; break;
		}
	}

	// translates mouse to controller

	void on(event_Mouse& e)
	{
		vec2 target = vec2(e.screen_x, e.screen_y) * GetModule<CoordTranslation>().ScreenToWorld;

		SendNow(event_Input{ InputName::AIM_X, target.x });
		SendNow(event_Input{ InputName::AIM_Y, target.y });
		SendNow(event_Input{ InputName::ATTACK, (float)e.button_left });
	}

	void on(event_Item_Pickup& e)
	{
		if (!playerEntity.IsAlive())      return;
		if (e.sinkEntity != playerEntity) return;

		Player& player = playerEntity.Get<Player>();

		switch (e.item)
		{
			case ITEM_HEALTH: 
			{
				Send(event_Sand_HealCell{ e.sinkEntity, -1 }); 
				break;
			}

			case ITEM_WEAPON_CANNON:
			{
				player.CurrentWeapon = WEAPON_CANNON;
				player.CurrentWeaponAmmo = 0;
				player.CurrentWeaponInaccuracy = .01;
				player.CurrentWeaponAttackTime = .4;
				break;
			}

			case ITEM_WEAPON_MINIGUN: 
			{
				player.CurrentWeapon = WEAPON_MINIGUN;
				player.CurrentWeaponAmmo = 200;
				player.CurrentWeaponInaccuracy = .1;
				player.CurrentWeaponAttackTime = .04;
				break;
			}

			case ITEM_WEAPON_WATTZ:
			{
				player.CurrentWeapon = WEAPON_WATTZ;
				player.CurrentWeaponAmmo = 10;
				player.CurrentWeaponInaccuracy = 0;
				player.CurrentWeaponAttackTime = 1.f;
				break;
			}

			case ITEM_WEAPON_BOLTZ:
			{
				player.CurrentWeapon = WEAPON_BOLTZ;
				player.CurrentWeaponAmmo = 1000;
				player.CurrentWeaponInaccuracy = 1;
				player.CurrentWeaponAttackTime = .01;
				break;
			}

			case ITEM_REGOLITH:
			{
				player.Score += 1;
				break;
			}
		}
	}
};
