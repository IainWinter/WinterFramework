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
		background = GetPrefab_Texture("ui_playerHUD.png");
		background->SendToDevice();

		playerEntity = FirstEntityWith<Player>();
		playerSprite = playerEntity.Get<Sprite>().source;
	}

	void UI()
	{
		ImGui::SetNextWindowPos(ImVec2(200, 0));
		ImGui::Begin("asdasd");

		//ImGui::SliderFloat("asdasd", &fontOffset, -2, 2);
		ImGui::End();

		// sizes of elements

		vec2 screen = GetModule<Window>().Dimensions();
		vec2 hud = screen * vec2(.15, 1);
		vec2 scale = hud / background->Dimensions();
		vec2 health = vec2(hud.x, hud.x) * .5f;

		// health sprite is 16x16, .15 of screen * .5 = 192 or 12x16

		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::SetNextWindowSize(ImVec2(hud.x, hud.y));
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding,    ImVec2(0, 0));
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
		vec2 score = vec2(15, 52) * scale - vec2(0, text / 2.f + 2.f); // +2 is an adjustment to line the font up with the bg
		vec2 ammo  = vec2(15, 80) * scale - vec2(0, text / 2.f + 2.f); // I think that this doesnt need to happen if imgui aa is disabled

		ImGui::SetCursorPos(ImVec2(0, 0));
		ImGui::Image((void*)background->DeviceHandle(), ImVec2(hud.x, hud.y));

		ImGui::SetCursorPos(ImVec2((hud.x - health.x) / 2.f, (hud.y - health.y) / 2.f));
		ImGui::Image((void*)playerSprite->DeviceHandle(), ImVec2(health.x, health.y), ImVec2(0, 1), ImVec2(1, 0));

		ImGui::SetCursorPos(ImVec2(ammo.x, ammo.y));
		ImGui::Text("%d", GetAmmoCount());

		ImGui::SetCursorPos(ImVec2(score.x, score.y));
		ImGui::Text("%d", GetScoreCount());

		// this is going to be a window that displays the health

		ImGui::End();
		ImGui::PopStyleVar(2);
		ImGui::PopStyleColor(1);
	}

	int GetAmmoCount() const
	{
		return playerEntity.IsAlive() 
			? playerEntity.Get<Player>().CurrentWeaponAmmo 
			: 0;
	}

	int GetScoreCount() const
	{
		return playerEntity.IsAlive()
			? playerEntity.Get<Player>().Score
			: 0;
	}
};