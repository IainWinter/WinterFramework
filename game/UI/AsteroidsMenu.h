#pragma once

#include "app/System.h"
#include "Windowing.h"
#include "app/FontMap.h"
#include "Prefabs.h"
#include "Components/Player.h"
#include "Events.h"
#include <sstream>

struct System_UI_AsteroidsMenu : System<System_UI_AsteroidsMenu>
{
	void Init()
	{
	}

	void UI()
	{
		vec2 screen = GetWindow().Dimensions();
		vec2 midpoint = screen / 2.f;
		float scale = screen.y / 720;

		float healthSize = 50.f * scale;
		float margin = 10.f * scale;

		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::SetNextWindowSize(ImVec2(screen.x, screen.y));
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.f);
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 1, 1, 1));

		ImGui::Begin("Asteroids Menu", 0,
			ImGuiWindowFlags_NoResize
			| ImGuiWindowFlags_NoBackground
			| ImGuiWindowFlags_NoTitleBar
			| ImGuiWindowFlags_NoScrollbar
		);

		ImGui::SetWindowFontScale(scale / 2.f);

		ImGui::PushFont(FontMap::Get("Main Menu Title"));
		ImVec2 titleSize = ImGui::CalcTextSize("REGOLITH");
		ImGui::SetCursorPos(ImVec2(midpoint.x - titleSize.x / 2, midpoint.y - titleSize.y / 2));
		ImGui::Text("REGOLITH");
		ImGui::PopFont();

		ImGui::PushFont(FontMap::Get("Main Menu Button"));
		ImVec2 playSize = ImGui::CalcTextSize("Play");
		ImGui::SetCursorPosX(midpoint.x - playSize.x / 2);
		if (ImGui::Button("Play"))
		{
			SendToRoot(event_PlayGame{});
		}
		ImGui::PopFont();

		//ImVec2 highscoreSize = ImGui::CalcTextSize("Highscores");
		//ImGui::SetCursorPosX(midpoint.x - highscoreSize.x / 2);
		//ImGui::Button("Highscores");

		ImGui::End();
		ImGui::PopStyleVar(2);
		ImGui::PopStyleColor(1);
	}
};