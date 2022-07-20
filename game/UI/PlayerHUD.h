#pragma once

#include "app/System.h"
#include "Windowing.h"
#include "imgui/imgui.h"
#include "Prefabs.h"
#include "Components/Player.h"
#include "Sand/DiscreteSandWorld.h"
#include "Components/FuelTank.h"

struct System_UI_PlayerHUD : System<System_UI_PlayerHUD>
{
	r<Texture> background;
	r<Texture> playerSprite;
	Entity playerEntity;
	r<Texture> laserTank;

	void Init()
	{
		background = GetPrefab_Texture("ui_playerHUD.png");
		background->SendToDevice();

		playerEntity = FirstEntityWith<Player>();
		playerSprite = playerEntity.Get<Sprite>().source;
		laserTank = FirstEntityWith<LaserTank>().Get<FuelTank>().cells.display;
	}

	void UI()
	{
		vec2 screen = GetModule<Window>().Dimensions();
		vec2 size_hud = screen * vec2(.15, 1);
		vec2 scale = size_hud / background->Dimensions();

		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::SetNextWindowSize(ImVec2(size_hud.x, size_hud.y));
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.f);
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 1, 1, 1));

		UIFontScope font = GetModule<Window>().Fonts().Use("Pixel");

		ImGui::Begin("Player HUD", 0, 
			  ImGuiWindowFlags_NoResize 
			| ImGuiWindowFlags_NoInputs 
			| ImGuiWindowFlags_NoBackground 
			| ImGuiWindowFlags_NoTitleBar
			| ImGuiWindowFlags_NoScrollbar
		);

		ImGui::SetWindowFontScale(scale.y / 4.f);

		float text = ImGui::CalcTextSize("0").y;
		
		vec2 pos_score  = vec2(15, 52) * scale - vec2(0, text / 2.f + 2.f); // +2 is an adjustment to line the font up with the bg
		vec2 pos_ammo   = vec2(15, 80) * scale - vec2(0, text / 2.f + 2.f); // I think that this doesnt need to happen if imgui aa is disabled
		vec2 pos_health = vec2(18, 117) * scale;
		vec2 pos_laser  = vec2(16, 186) * scale;
		
		vec2 size_health = vec2(36.f, 36.f) * scale;
		vec2 size_laser  = vec2(40.f, 40.f) * scale;

		ImGui::SetCursorPos(ImVec2(0, 0));
		ImGui::Image((void*)background->DeviceHandle(), ImVec2(size_hud.x, size_hud.y));

		ImGui::SetCursorPos(ImVec2(pos_health.x, pos_health.y)); // (size_hud.x - size_health.x) / 2.f, (size_hud.y - size_health.y) / 2.f
		ImGui::Image((void*)playerSprite->DeviceHandle(), ImVec2(size_health.x, size_health.y), ImVec2(0, 1), ImVec2(1, 0));

		ImGui::SetCursorPos(ImVec2(pos_ammo.x, pos_ammo.y));
		ImGui::Text("%d", GetAmmoCount());

		ImGui::SetCursorPos(ImVec2(pos_score.x, pos_score.y));
		ImGui::Text("%d", GetScoreCount());

		ImGui::SetCursorPos(ImVec2(pos_laser.x, pos_laser.y));
		ImGui::Image((void*)laserTank->DeviceHandle(), ImVec2(size_laser.x, size_laser.y));

		// this is going to be a window that displays the health

		ImGui::End();
		ImGui::PopStyleVar(2);
		ImGui::PopStyleColor(1);
	}

	int GetAmmoCount() const
	{
		return playerEntity.IsAlive() 
			? playerEntity.Get<Player>().Current.Ammo 
			: 0;
	}

	int GetScoreCount() const
	{
		return playerEntity.IsAlive()
			? playerEntity.Get<Player>().Score
			: 0;
	}

	bool TakeLaserFuel() const
	{
		return playerEntity.IsAlive()
			&& playerEntity.Get<Player>().AttackFireInputAlt;
	}
};