#pragma once

#include "app/System.h"
#include "Windowing.h"
#include "app/FontMap.h"
#include "Prefabs.h"
#include "Components/Player.h"

struct System_UI_AsteroidsHUD : System<System_UI_AsteroidsHUD>
{
	r<Texture> playerSprite;
	int lastPlayerLives = 3;

	void Init()
	{
		playerSprite = mkr<Texture>(_A("player.png"));
		playerSprite->SendToDevice();
	}

	void UI()
	{
		vec2 screen = GetModule<Window>().Dimensions();
		vec2 size_hud = screen * vec2(.15, 1);

		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::SetNextWindowSize(ImVec2(size_hud.x, size_hud.y));
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.f);
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 1, 1, 1));

		ImGui::PushFont(GetModule<FontMap>().Get("Score"));

		ImGui::Begin("Player HUD", 0,
			ImGuiWindowFlags_NoResize
			| ImGuiWindowFlags_NoInputs
			| ImGuiWindowFlags_NoBackground
			| ImGuiWindowFlags_NoTitleBar
			| ImGuiWindowFlags_NoScrollbar
		);

		ImGui::SetCursorPos(ImVec2(10, 10));
		ImGui::Text("%d", GetScoreCount());

		float healthSize = 50.f;

		for (int i = 0; i < GetPlayerLives(); i++)
		{
			ImGui::SetCursorPos(ImVec2(10 + (10 + healthSize) * i, 10 + 10 + ImGui::CalcTextSize("0").y));
			ImGui::Image((void*)playerSprite->DeviceHandle(), ImVec2(healthSize, healthSize), ImVec2(0, 1), ImVec2(1, 0));
		}

		ImGui::End();
		ImGui::PopStyleVar(2);
		ImGui::PopStyleColor(1);
		ImGui::PopFont();
	}

	int GetScoreCount() const
	{
		Entity player = FirstEntityWith<Player>();
		return player.IsAlive() ? player.Get<Player>().Score : 0;
	}

	int GetPlayerLives()
	{
		Entity player = FirstEntityWith<Player>();
		lastPlayerLives = player.IsAlive() ? player.Get<Player>().Lives : lastPlayerLives;
		return lastPlayerLives;
	}
};