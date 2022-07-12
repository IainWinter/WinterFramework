#pragma once

#include "Leveling.h"
#include "ext/Time.h"

struct System_Testing : SystemBase
{
	Entity entity;
	Entity target;

	void Init()
	{
		entity = CreateEntity();
		target = CreateEntity();

		entity.Add<Transform2D>(vec2(-10, 10));
		target.Add<Transform2D>(vec2( 10, 10));
	}

	void UI()
	{
		ImGui::Begin("indev");

		{
			ItemType item = (ItemType)-1;

			if (ImGui::Button("Spawn Health"))     item = ITEM_HEALTH;
			if (ImGui::Button("Spawn Energy"))     item = ITEM_ENERGY;
			if (ImGui::Button("Spawn Rgolith"))    item = ITEM_REGOLITH;
			if (ImGui::Button("Spawn Core Shard")) item = ITEM_CORE_SHARD;
			if (ImGui::Button("Spawn Minigun"))    item = ITEM_WEAPON_MINIGUN;
			if (ImGui::Button("Spawn Boltz"))      item = ITEM_WEAPON_BOLTZ;
			if (ImGui::Button("Spawn Wattz"))      item = ITEM_WEAPON_WATTZ;

			if (item != -1)
			{
				Send(event_Item_Spawn{ item, vec2(10, 0), 1});
			}
		}

		ImGui::Separator();

		{
			Weapon weapon = (Weapon)-1;

			if (ImGui::Button("Fire Cannon"))    weapon = WEAPON_CANNON;
			if (ImGui::Button("Fire Laser"))     weapon = WEAPON_LASER;
			if (ImGui::Button("Fire Minigun"))   weapon = WEAPON_MINIGUN;
			if (ImGui::Button("Fire Boltz"))     weapon = WEAPON_BOLTZ;
			if (ImGui::Button("Fire Wattz"))     weapon = WEAPON_WATTZ;
			if (ImGui::Button("Fire Fuel Shot")) weapon = WEAPON_FUEL_SHOT;

			if (weapon != -1)
			{
				Send(event_FireWeapon{ entity, target, weapon });
			}
		}

		ImGui::Separator();

		{
			EnemyType enemy = (EnemyType)-1;

			if (ImGui::Button("Spawn Fighter")) enemy = ENEMY_FIGHTER;
			if (ImGui::Button("Spawn Bomb"))    enemy = ENEMY_BOMB;
			if (ImGui::Button("Spawn Station")) enemy = ENEMY_STATION;
			if (ImGui::Button("Spawn Base"))    enemy = ENEMY_BASE;

			if (enemy != -1)
			{
				Send(event_Enemy_Spawn{ enemy, vec2(10, 0), true });
			}
		}

		ImGui::End();
	}
};