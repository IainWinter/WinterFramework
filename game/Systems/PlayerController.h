#pragma once

#include "Leveling.h"
#include "CoordTranslation.h"
#include "Windowing.h"
#include "Sand/Sand.h"
#include "Events.h"
#include "ext/rendering/Particle.h"
#include "Components/Player.h"
#include "Components/EnemyAI.h"
#include "Components/FuelTank.h"
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

		playerEntity.Get<Player>().Alt = { WEAPON_LASER_LARGE, 0, 0, .001f };

		Send(event_Item_Pickup{ playerEntity, ITEM_WEAPON_CANNON });
	}

	void Update()
	{
		if (!playerEntity.IsAlive()) return;

		Player& player = playerEntity.Get<Player>();

		player.m_attackTimer    -= Time::DeltaTime();
		player.m_attackTimerAlt -= Time::DeltaTime();
		
		FirstEntityWith<LaserTank>().Get<FuelTank>().openOutlet = player.AttackFireInputAlt;

		if (   player.AttackFireInputAlt 
			&& player.m_attackTimerAlt <= 0.f 
			&& player.AttackFuelAlt > 0)
		{
			player.m_attackTimerAlt = player.Alt.AttackTime;
			player.AttackFuelAlt -= player.AttackFuelConsumptionAlt;
			Send(event_FireWeapon{ playerEntity, target, player.Alt.Weapon, player.Alt.Inaccuracy });
		}

		else 
		if (   player.AttackFireInput 
			&& player.m_attackTimer <= 0.f)
		{
			player.m_attackTimer = player.Current.AttackTime;
			Send(event_FireWeapon{ playerEntity, target, player.Current.Weapon, player.Current.Inaccuracy });

			if (player.Current.Weapon != WEAPON_CANNON)
			{
				player.Current.Ammo -= 1;
				if (player.Current.Ammo <= 0)
				{
					Send(event_Item_Pickup{ playerEntity, ITEM_WEAPON_CANNON });
				}
			}
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


		SendNow(event_Item_Pickup{ playerEntity, ITEM_ENERGY }); // debug
	}

	void on(event_Input& e)
	{
		if (!playerEntity.IsAlive()) return;

		Player& player = playerEntity.Get<Player>();
		switch (e.name)
		{
			case InputName::UP:    player.MovementInput.y += e.state; break;
			case InputName::DOWN:  player.MovementInput.y -= e.state; break;
			case InputName::RIGHT: player.MovementInput.x += e.state; break;
			case InputName::LEFT:  player.MovementInput.x -= e.state; break;

			case InputName::AIM_X:      player.AttackLocationInput.x =   e.state; break;
			case InputName::AIM_Y:      player.AttackLocationInput.y =   e.state; break;
			case InputName::ATTACK:     player.AttackFireInput       = !!e.state; break;
			case InputName::ATTACK_ALT: player.AttackFireInputAlt    = !!e.state; break;
		}
	}

	// translates mouse to controller

	void on(event_Mouse& e)
	{
		vec2 target = vec2(e.screen_x, e.screen_y) * GetModule<CoordTranslation>().ScreenToWorld;

		SendNow(event_Input{ InputName::AIM_X, target.x });
		SendNow(event_Input{ InputName::AIM_Y, target.y });
		SendNow(event_Input{ InputName::ATTACK,     (float)e.button_left });
		SendNow(event_Input{ InputName::ATTACK_ALT, (float)e.button_right });
	}

	void on(event_Item_Pickup& e)
	{
		if (!playerEntity.IsAlive())      return;
		if (e.sinkEntity != playerEntity) return;

		Player& player = playerEntity.Get<Player>();

		switch (e.item)
		{
			case ITEM_WEAPON_CANNON:
			{
				player.Current = { WEAPON_CANNON, 0, .01f, .4f };
				break;
			}

			case ITEM_WEAPON_MINIGUN: 
			{
				player.Current = { WEAPON_MINIGUN, 200, .1f, .04f };
				break;
			}

			case ITEM_WEAPON_WATTZ:
			{
				player.Current = { WEAPON_WATTZ, 10, 0, 1.0f };
				break;
			}

			case ITEM_WEAPON_BOLTZ:
			{
				player.Current = { WEAPON_BOLTZ, 1000, 1, .01f };
				break;
			}

			case ITEM_REGOLITH:
			{
				player.Score += 1;
				break;
			}

			case ITEM_HEALTH: 
			{
				Send(event_Sand_HealCell{ e.sinkEntity, -1 }); 
				break;
			}

			case ITEM_ENERGY:
			{
				FirstEntityWith<LaserTank>().Get<FuelTank>().feed.push_back(Color(255, 0, 0));
				break;
			}
		}
	}
};

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