#pragma once

#include "app/System.h"
#include "Sand/SandEntity.h"
#include "Components/Player.h"

struct System_PlayerSpawner : System<System_PlayerSpawner>
{
private:
	int m_lives = 3;
	float m_respawnTime = 2.f;
	float m_respawnTimer = 2.f;

public:
	void Update()
	{
		if (!IsPlayerAlive() && m_lives > 0)
		{
			m_respawnTimer -= Time::DeltaTime();
			if (m_respawnTimer < 0.f)
			{
				SpawnPlayer();
				m_lives -= 1;
				m_respawnTimer = m_respawnTime;
			}
		}
	}

private:

	bool IsPlayerAlive() const
	{
		return FirstEntityWith<Player>().IsAlive();
	}

	void SpawnPlayer()
	{
		Entity playerEntity = CreateSandSprite("player.png", "player_collider_mask.png");
		//player.Add<KeepOnScreen>();
		playerEntity.Add<ItemSink>();
		playerEntity.Add<SandHealable>();
		playerEntity.Add<SandDieInTimeWithLowCoreCount>();

		// this is technically wrong, should be WhileColliding

		playerEntity.Add<Rigidbody2D>().OnCollision += [=](CollisionInfo info)
		{
			info.me.Get<Player>().Lives -= 1; // update UI

			event_Sand_ExplodeToDust e;
			e.entity = playerEntity;
			e.putColliderOnDust = true;

			e.onCreate = [this](Entity e)
			{
				e.Remove<Item>();
				e.Add<DestroyInTime>(m_respawnTime);
			};

			Send(e);
		};

		Player& player = playerEntity.Add<Player>();
		player.Alt = { WEAPON_LASER_LARGE, 0, 0, .001f }; // big laser
		player.Lives = m_lives;

		Send(event_Item_Pickup{ playerEntity, ITEM_WEAPON_CANNON });

		playerEntity.Get<SandSprite>().invulnerable = true;
		playerEntity.Get<Rigidbody2D>().SetEnableCollision(false);

		timer = 3.f;

		Coroutine([playerEntity, this]() 
		{
			timer -= Time::DeltaTime();
			bool done = timer < 0.f;

			if (done)
			{
				playerEntity.Get<Rigidbody2D>().SetEnableCollision(true);
				playerEntity.Get<SandSprite>().invulnerable = false;
				playerEntity.Get<Sprite>().tint = Color(255, 255, 255, 255);
			}

			else
			{
				flickerTimer -= Time::DeltaTime();
				if (flickerTimer < 0.f)
				{
					flickerTimer = .1f;

					u8& alpha = playerEntity.Get<Sprite>().tint.a;

					if (alpha == 0) alpha = 255;
					else            alpha = 0;
				}
			}

			return done;
		});
	}

	float timer = 3.f;
	float flickerTimer = .1f;
};