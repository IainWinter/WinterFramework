#pragma once

#include "Windowing.h"
#include "app/System.h"
#include "app/FontMap.h"
#include "ext/Time.h"

struct System_UI_PauseMenu : System<System_UI_PauseMenu>
{
	float offset = -1000;

	void UI()
	{
		vec2 screen = GetWindow().Dimensions();
		vec2 pause = screen * vec2(.4, 1);
		
		offset = lerp(offset, screen.x * .15f, Time::RawDeltaTime() * 20.f);

		ImGui::SetNextWindowPos(ImVec2(offset, 0));
		ImGui::SetNextWindowSize(ImVec2(pause.x, pause.y));
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(20, 20));
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.f);
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 1, 1, 1));
		ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0, 0, 0, .4f));
		ImGui::PushFont(FontMap::Get("Pixel Title"));

		ImGui::Begin("Pause Menu", 0, 
				ImGuiWindowFlags_NoResize 
			| ImGuiWindowFlags_NoTitleBar
			| ImGuiWindowFlags_NoScrollbar
		);

		ImGui::PushFont(FontMap::Get("Pixel"));
		ImGui::Text("Paused");
		ImGui::PopFont();

		ImGui::End();
		ImGui::PopStyleVar(2);
		ImGui::PopStyleColor(2);
		ImGui::PopFont();
	}
};