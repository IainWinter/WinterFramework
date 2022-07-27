#pragma once

#include "app/System.h"
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

	// these need to be here to reset on death
	vec2 MovementInput;
	vec2 AttackLocationInput;
	float AttackFireInput;
	float AttackFireInputAlt;

	void Init()
	{
		Attach<event_Input>();
		//Attach<event_Mouse>();
		Attach<event_Item_Pickup>();

		target = CreateEntity().AddAll(Transform2D(vec2(0.f, 0.f)));
	}

	void Update()
	{
		playerEntity = FirstEntity<Player>();
		if (!playerEntity.IsAlive()) return;

		Player& player = playerEntity.Get<Player>();

		if (player.AmmoRechargeTimer < 0.f)
		{
			player.AmmoRechargeTimer += Time::DeltaTime();

			if (player.AmmoRechargeTimer >= 0.f)
			{
				player.Ammo = 5;
			}
		}

		/*if (player.AmmoRechargeTimer > player.AmmoRechargeTime)
		{
			player.AmmoRechargeTimer = 0.f;
			player.Ammo = clamp(player.Ammo + 1, 0, player.AmmoMax);
		}*/

		player.m_attackTimer    -= Time::DeltaTime();
		player.m_attackTimerAlt -= Time::DeltaTime();
		
		//FirstEntityWith<LaserTank>().Get<FuelTank>().openOutlet = player.AttackFireInputAlt;

		//if (   AttackFireInputAlt 
		//	&& player.m_attackTimerAlt <= 0.f 
		//	&& player.AttackFuelAlt > 0)
		//{
		//	player.m_attackTimerAlt = player.Alt.AttackTime;
		//	player.AttackFuelAlt -= player.AttackFuelConsumptionAlt;
		//	Send(event_FireWeapon{ playerEntity, target, player.Alt.Weapon, player.Alt.Inaccuracy });
		//}

		//else 
		if (   AttackFireInput 
//			&& player.Ammo > 0
			&& player.m_attackTimer <= 0.f)
		{
//			AttackFireInput = false;
//			player.Ammo -= 1;
			player.m_attackTimer = player.Current.AttackTime;

//			if (player.Ammo == 0) // second cooldown if shot all bullets
//			{
//				player.AmmoRechargeTimer = -1;
//			}
			
			Send(event_FireWeapon{ playerEntity, target, player.Current.Weapon, player.Current.Inaccuracy });

			//if (player.Current.Weapon != WEAPON_CANNON)
			//{
			//	player.Current.Ammo -= 1;
			//	if (player.Current.Ammo <= 0)
			//	{
			//		if (player.Ammo == 0) // second cooldown if shot all bullets
			//		{
			//			player.AmmoRechargeTimer = -1;
			//		}

			//		Send(event_Item_Pickup{ playerEntity, ITEM_WEAPON_CANNON });
			//	}
			//}
		}

		target.Get<Transform2D>().position = playerEntity.Get<Transform2D>().position + PlayerHeading(); // shoot forward
	}

	vec2 PlayerHeading() const
	{
		float angle = playerEntity.Get<Rigidbody2D>().GetAngle();
		return vec2(cos(angle + wPI / 2), sin(angle + wPI / 2)); // sprite heading is 90 above real heading
	}

	void FixedUpdate()
	{
		if (!playerEntity.IsAlive()) return;

		auto [player, body] = playerEntity.GetAll<Player, Rigidbody2D>();

		// tank controls

		float moveForward = MovementInput.y;
		if (moveForward == -1.f) moveForward = 0.f; // dont allow moving backwards

		float angle = body.GetAngle() + -MovementInput.x * player.RotationSpeed * Time::FixedTime(); // radians per second
		vec2 force = PlayerHeading() * player.MovementSpeed * moveForward;

		body.SetAngle(angle);
		body.ApplyForce(force);
		body.SetAngularVelocity(0);

		// absolute controls

		//vec2 vel = lerp(
		//	body.GetVelocity(),
		//	safe_normalize(player.MovementInput) * player.MovementSpeed,
		//	Time::DeltaTime() * player.MovementAccelerationScaleFactor);

		//body.SetVelocity(vel);
	}

	void on(event_Input& e)
	{
		switch (e.name)
		{
			case InputName::UP:    MovementInput.y += e.state; break;
			case InputName::DOWN:  MovementInput.y -= e.state; break;
			case InputName::RIGHT: MovementInput.x += e.state; break;
			case InputName::LEFT:  MovementInput.x -= e.state; break;

			case InputName::AIM_X:      AttackLocationInput.x  = e.state; break;
			case InputName::AIM_Y:      AttackLocationInput.y  = e.state; break;
			case InputName::ATTACK:     AttackFireInput       += e.state; break;
			case InputName::ATTACK_ALT: AttackFireInputAlt    += e.state; break;
		}
	}

	// translates mouse to controller

	//void on(event_Mouse& e)
	//{
	//	vec2 target = vec2(e.screen_x, e.screen_y) * GetModule<CoordTranslation>().ScreenToWorld;

	//	SendNow(event_Input{ InputName::AIM_X, target.x });
	//	SendNow(event_Input{ InputName::AIM_Y, target.y });
	//	SendNow(event_Input{ InputName::ATTACK,     (float)e.button_left });
	//	SendNow(event_Input{ InputName::ATTACK_ALT, (float)e.button_right });
	//}

	void on(event_Item_Pickup& e)
	{
		if (!playerEntity.IsAlive())      return;
		if (e.sinkEntity != playerEntity) return;

		Player& player = playerEntity.Get<Player>();

		switch (e.item)
		{
			case ITEM_WEAPON_CANNON:
			{
				player.Current = { WEAPON_CANNON, 0, .01f, .1f };
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
				Send(event_AddScore{ 1 });
				break;
			}

			case ITEM_HEALTH: 
			{
				Send(event_Sand_HealCell{ e.sinkEntity, -1 }); 
				break;
			}

			//case ITEM_ENERGY:
			//{
			//	FirstEntityWith<LaserTank>().Get<FuelTank>().feed.push_back(Color(255, 0, 0));
			//	break;
			//}
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