#pragma once

#include "Leveling.h"
#include "Prefabs.h"
#include "imgui/imgui.h"
#include "Components/Player.h"

struct System_UI_PlayerHUD : System<System_UI_PlayerHUD>
{
	r<Texture> background;
	r<Texture> playerSprite;
	Entity playerEntity;

	void Init()
	{
		background = GetPrefab_Texture("mud.png");
		background->SendToDevice();

		playerEntity = FirstEntityWith<Player>();
		playerSprite = playerEntity.Get<Sprite>().source;
	}

	void UI()
	{
		vec2 screen = GetModule<Window>().Dimensions();
		vec2 hud = screen * vec2(.2, 1.f);
		vec2 health = vec2(hud.x, hud.x) * .5f;

		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::SetNextWindowSize(ImVec2(hud.x, hud.y));
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding,    ImVec2(0, 0));
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.f);
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 1, 1, 1));

		ImGui::Begin("Player HUD", 0, 
			  ImGuiWindowFlags_NoResize 
			| ImGuiWindowFlags_NoInputs 
			| ImGuiWindowFlags_NoBackground 
			| ImGuiWindowFlags_NoTitleBar
			| ImGuiWindowFlags_NoScrollbar
		);

		ImGui::SetCursorPos(ImVec2(0, 0));
		ImGui::Image((void*)background->DeviceHandle(), ImVec2(hud.x, hud.y));

		ImGui::SetCursorPos(ImVec2((hud.x - health.x) / 2.f, (hud.y - health.y) / 2.f));
		ImGui::Image((void*)playerSprite->DeviceHandle(), ImVec2(health.x, health.y), ImVec2(0, 1), ImVec2(1, 0));

		ImGui::SetCursorPos(ImVec2(20, 20));
		ImGui::Text("Ammo %d", GetAmmoCount());

		// this is going to be a window that displays the health

		ImGui::End();
		ImGui::PopStyleVar(2);
		ImGui::PopStyleColor(1);
	}

	int GetAmmoCount()
	{
		return playerEntity.IsAlive() 
			? playerEntity.Get<Player>().CurrentWeaponAmmo 
			: 0;
	}
};