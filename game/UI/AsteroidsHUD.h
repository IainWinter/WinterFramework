#pragma once

#include "app/System.h"
#include "Windowing.h"
#include "app/FontMap.h"
#include "Prefabs.h"
#include "Components/Player.h"
#include "Events.h"
#include <sstream>

int FilterAZ(ImGuiInputTextCallbackData* data)
{
	ImWchar c = data->EventChar;
	if (c >= 'A' && c <= 'Z') return 0;
	return 1;
}

struct System_UI_AsteroidsHUD : System<System_UI_AsteroidsHUD>
{
	r<Texture> playerSprite;
	int lastPlayerLives = 3;
	int lastPlayerScore;
	
	char name[6];

	float m_gameoverTimer;

	float m_fadeTarget;
	float m_currentFade;

	void Init()
	{
		playerSprite = mkr<Texture>(_A("player.png"));
		playerSprite->SendToDevice();

		m_gameoverTimer = 0.f;
		m_currentFade = 0.0f;
		m_fadeTarget = 0.5f;   // game over fade amount
	}

	void UI()
	{
		vec2 screen = GetModule<Window>().Dimensions();
		vec2 midpoint = screen / 2.f;
		float scale = screen.y / 720;

		float healthSize = 50.f * scale;
		float margin = 10.f * scale;

		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::SetNextWindowSize(ImVec2(screen.x, screen.y));
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.f);
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 1, 1, 1));

		ImGui::PushFont(GetModule<FontMap>().Get("Score"));

		ImGui::Begin("Asteroids HUD", 0,
			ImGuiWindowFlags_NoResize
			| ImGuiWindowFlags_NoBackground
			| ImGuiWindowFlags_NoTitleBar
			| ImGuiWindowFlags_NoScrollbar
		);

		ImGui::SetWindowFontScale(scale / 2.f);

		if (IsGameOver())
		{
			m_currentFade = lerp(m_currentFade, m_fadeTarget, Time::DeltaTime());

			ImGui::GetBackgroundDrawList()->AddRectFilled(
				ImVec2(0, 0),
				ImVec2(screen.x, screen.y),
				Color(0, 0, 0, m_currentFade * 255).as_u32
			);

			m_gameoverTimer += Time::DeltaTime();

			if (m_gameoverTimer < 3.f)
			{
				ImGui::PushFont(GetModule<FontMap>().Get("Game Over"));
				
				const char* text = "GAME OVER";
				ImVec2 size = ImGui::CalcTextSize(text);
				
				ImGui::SetCursorPos(ImVec2(
					midpoint.x      - size.x / 2.f,
					screen.y * .33f - size.y / 2.f)
				);

				ImGui::Text("GAME OVER");
				ImGui::PopFont();
			}

			else
			{
				ImGui::PushFont(GetModule<FontMap>().Get("Final Score"));

				const char* text = "ENTER YOUR NAME";
				ImVec2 size = ImGui::CalcTextSize(text);
				
				ImGui::PushStyleColor(ImGuiCol_FrameBg,       ImVec4(0, 0, 0, 0));
				ImGui::PushStyleColor(ImGuiCol_FrameBgActive, ImVec4(0, 0, 0, 0));

				ImGui::SetCursorPosY(screen.y * .33f - size.y / 2.f);

				ImGui::SetCursorPosX(midpoint.x - size.x / 2.f);
				ImGui::Text(text);

				ImGui::Spacing();
				ImGui::Spacing();
				ImGui::Spacing();
				ImGui::Spacing();
				ImGui::Spacing();

				ImGui::SetCursorPosX(midpoint.x - size.x / 2.f);
				ImGui::InputText("##", name, sizeof(name), ImGuiInputTextFlags_CharsNoBlank | ImGuiInputTextFlags_CharsUppercase | ImGuiInputTextFlags_CallbackCharFilter, FilterAZ);

				ImGui::Spacing();
				ImGui::Spacing();
				ImGui::Spacing();
				ImGui::Spacing();
				ImGui::Spacing();
				ImGui::Spacing();
				ImGui::Spacing();
				ImGui::Spacing();
				ImGui::Spacing();
				ImGui::Spacing();

				ImGui::PushFont(GetModule<FontMap>().Get("OK Button"));
				ImGui::SetCursorPosX(midpoint.x - size.x / 2.f);
				
				if (ImGui::Button("OK"))
				{
					SendToRoot(event_SubmitHighscore{ std::string(name), lastPlayerScore });
					m_fadeTarget = 0.f;
				}
				
				ImGui::PopFont();

				ImGui::PopStyleColor(2);
				ImGui::PopFont();
			}
		}

		ImGui::SetCursorPos(ImVec2(margin, margin));
		ImGui::Text("%d", GetScoreCount());

		for (int i = 0; i < GetPlayerLives(); i++)
		{
			ImGui::SetCursorPos(ImVec2(margin + (margin + healthSize) * i, margin + margin + ImGui::CalcTextSize("0").y));
			ImGui::Image((void*)playerSprite->DeviceHandle(), ImVec2(healthSize, healthSize), ImVec2(0, 1), ImVec2(1, 0));
		}

		ImGui::End();
		ImGui::PopStyleVar(2);
		ImGui::PopStyleColor(1);
		ImGui::PopFont();
	}

	int GetScoreCount()
	{
		Entity player = FirstEntityWith<Player>();
		lastPlayerScore = player.IsAlive() ? player.Get<Player>().Score : lastPlayerScore;
		return lastPlayerScore;
	}

	int GetPlayerLives()
	{
		Entity player = FirstEntityWith<Player>();
		lastPlayerLives = player.IsAlive() ? player.Get<Player>().Lives : lastPlayerLives;
		return lastPlayerLives;
	}

	bool IsGameOver() const
	{
		return lastPlayerLives == 0 || true;
	}
};