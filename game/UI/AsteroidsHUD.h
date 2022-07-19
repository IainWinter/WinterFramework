#pragma once

#include "Leveling.h"
#include "Windowing.h"
#include "imgui/imgui.h"
#include "Prefabs.h"
#include "Components/Player.h"

struct System_UI_AsteroidsHUD : System<System_UI_AsteroidsHUD>
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

		ImGui::SetCursorPos(ImVec2(pos_score.x, pos_score.y));
		ImGui::Text("%d", GetScoreCount());

		ImGui::End();
		ImGui::PopStyleVar(2);
		ImGui::PopStyleColor(1);
	}

	int GetScoreCount() const
	{
		return playerEntity.IsAlive()
			? playerEntity.Get<Player>().Score
			: 0;
	}
};