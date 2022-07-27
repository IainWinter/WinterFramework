#pragma once

#include "app/System.h"
#include "Windowing.h"
#include "app/FontMap.h"
#include "Prefabs.h"
#include "Components/Player.h"
#include "Events.h"
#include <sstream>

float easeInOutCubic(float x)
{
	return x < 0.5 ? 4 * x * x * x : 1 - pow(-2 * x + 2, 3) / 2;
}

struct HighscoreRecord
{
	int order;
	int score;
	std::string name;
};

struct System_UI_AsteroidsMenu : System<System_UI_AsteroidsMenu>
{
	int music;
	r<Texture> title_main;
	r<Texture> title_highscores;
	r<Texture> title_settings;

	std::vector<HighscoreRecord> records;

	void Init()
	{
		records =
		{
			{ 1, 10023, "Test" },
			{ 2,  9023, "Iain" },
			{ 3,  8420, "Another" },
			{ 4,   100, "Same" },
			{ 4,   100, "Same" },
			{ 4,   100, "Same" },
			{ 4,   100, "Same" },
			{ 4,   100, "Same" },
			{ 4,   100, "Same" },
			{ 4,   100, "Same" },
			{ 4,   100, "Same" },
			{ 4,   100, "Same" },
			{ 4,   100, "Same" },
			{ 4,   100, "Same" },
			{ 4,   100, "Same" },
			{ 4,   100, "Same" },
			{ 4,   100, "Same" },
			{ 4,   100, "Same" },
		};

		music = PlaySound("event:/music_menu_main");
		
		title_main       = GetPrefab_Texture("title_main.png");
		title_highscores = GetPrefab_Texture("title_highscores.png");
		title_settings   = GetPrefab_Texture("title_settings.png");
		
		title_main      ->SendToDevice();
		title_highscores->SendToDevice();
		title_settings  ->SendToDevice();
	}

	ImVec2 GetSize(const r<Texture>& texture, float height_pixels)
	{
		float ratio = texture->Width() / (float)texture->Height();
		float height = height_pixels * scale;
		float width = height * ratio;

		return ImVec2(width, height);
	}

	const vec2 highscoreLocation = vec2(-1500, 0);
	const vec2 settingsLocation  = vec2( 1500, 0);

	void PressedPlay()
	{
		StopSound(music);
		PlaySound("event:/menu_press_play");

		Delay(1.f, [this]() { SetTarget(vec2(0, -1000.f)); });
		Delay(3.f, [this]() { SendToRoot(event_PlayGame{}); });
	}

	void PressedExit()
	{
		SetTarget(vec2(0, 1000.f));
		SetTargetFade(1.f);
		Delay(1.f, [this]() { SendToRoot(event_Shutdown{});  });
	}

	void PressedBack()
	{
		SetTarget(vec2(0));
	}

	void PressedHighscores()
	{
		SetTarget(highscoreLocation);
	}

	void PressedSettings()
	{
		SetTarget(settingsLocation);
	}

	enum MenuState
	{
		MAIN,
		HIGHSCORES,
		SETTINGS,
		EXIT
	};

	vec2 screen, midpoint;
	float scale, margin;
	MenuState currentState = MAIN;

	vec2  menuAnchorPoint                = vec2(0.f);
	vec2  menuAnchorPointLast            = vec2(0.f);
	vec2  menuAnchorPointTarget          = vec2(0.f);
	float menuAnchorPointTransitionTimer = 1.f;

	float currentFade = 0.f;
	float targetFade  = 0.f;
	float lastFade    = 0.f;
	float fadeTimer   = 0.f;

	float highscorePos;
	float settingsPos;

	void SetTarget(vec2 target)
	{
		menuAnchorPointTarget = target;
		menuAnchorPointLast = menuAnchorPoint;
		menuAnchorPointTransitionTimer = 0.f;
	}

	void SetTargetFade(float fade)
	{
		targetFade = fade;
		lastFade = currentFade;
		fadeTimer = 0.f;
	}

	void UI()
	{
		screen   = GetWindow().Dimensions();
		midpoint = screen / 2.f;
		scale    = screen.y / 720;
		margin   = 10.f * scale;

		menuAnchorPointTransitionTimer += Time::DeltaTime();
		
		if (menuAnchorPointTransitionTimer > 1.f)
		{
			menuAnchorPoint = menuAnchorPointTarget;
		}

		else
		{
			menuAnchorPoint = lerp(menuAnchorPointLast, menuAnchorPointTarget, easeInOutCubic(menuAnchorPointTransitionTimer));
		}

		midpoint -= menuAnchorPoint * scale;

		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::SetNextWindowSize(ImVec2(screen.x, screen.y));

		ImGui::Begin("Asteroids Menu", 0,
			ImGuiWindowFlags_NoResize
			| ImGuiWindowFlags_NoBackground
			| ImGuiWindowFlags_NoTitleBar
			| ImGuiWindowFlags_NoScrollbar
		);

		fadeTimer += Time::DeltaTime();
		if (fadeTimer > 1.f)
		{
			currentFade = targetFade;
		}

		else
		{
			currentFade = lerp(lastFade, targetFade, easeInOutCubic(fadeTimer));
		}

		ImGui::GetBackgroundDrawList()->AddRectFilled(
			ImVec2(0, 0),
			ImVec2(screen.x, screen.y),
			Color(0, 0, 0, currentFade * 255).as_u32
		);

		ImGui::SetWindowFontScale(scale / 2.f);

		ImGui::PushFont(FontMap::Get("Main Menu Button"));

		Main(midpoint.x);
		
		if (menuAnchorPoint.x > 0)
		{
			Settings(midpoint.x + settingsLocation.x * scale);
		}

		else if (menuAnchorPoint.x < 0)
		{
			Highscores(midpoint.x + highscoreLocation.x * scale);
		}

		ImGui::PopFont();

		ImGui::End();
	}

	void Main(float position)
	{
		auto [width, height] = GetSize(title_main, 200);

		ImGui::SetCursorPos(ImVec2(position - width / 2, midpoint.y - height));
		ImGui::Image((void*)title_main->DeviceHandle(), ImVec2(width, height));

		ImGui::SetCursorPosY(midpoint.y + height / 2.f);

		if (MenuButton("Play",       MAIN))       PressedPlay();
		if (MenuButton("Highscores", HIGHSCORES)) PressedHighscores();
		if (MenuButton("Settings",   SETTINGS))   PressedSettings();
		if (MenuButton("Exit",       EXIT))       PressedExit();
	}

	void Highscores(float position)
	{
		auto [width, height] = GetSize(title_highscores, 100);

		ImGui::SetCursorPos(ImVec2(position - screen.x / 2 + margin * 2, margin * 2));
		ImGui::Image((void*)title_highscores->DeviceHandle(), ImVec2(width, height));

		float backWidth = ImGui::CalcTextSize("Back").x;

		ImGui::SetCursorPos(ImVec2(position + screen.x / 2.f - backWidth - 2 * margin, midpoint.y));
		if (ImGui::Button("Back"))
		{
			PressedBack();
		}

		// table

		float twidth  = screen.x * .5f;
		float theight = screen.y * .5f;

		ImGui::SetCursorPos(ImVec2(position - twidth / 2, screen.y * .33f));
		ImGui::PushFont(FontMap::Get("Highscore Table"));

		ImGui::SetNextWindowPos(ImVec2(position - twidth / 2, screen.y * .33f));
		ImGui::BeginChild("Highscores titles", ImVec2(twidth, -1));

		if (ImGui::BeginTable("Upgrades Titles", 3))
		{
			ImGui::TableSetupColumn("#",     ImGuiTableColumnFlags_WidthStretch, .1);
			ImGui::TableSetupColumn("Name",  ImGuiTableColumnFlags_WidthStretch);
			ImGui::TableSetupColumn("Score", ImGuiTableColumnFlags_WidthStretch, .5);
		
			ImGui::TableNextColumn(); ImGui::Text("#");
			ImGui::TableNextColumn(); ImGui::Text("Name");
			ImGui::TableNextColumn(); ImGui::Text("Score");

			ImGui::EndTable();
		}

		float headerSize = ImGui::GetItemRectSize().y + margin;

		ImGui::Separator();
		ImGui::EndChild();

		ImGui::SetNextWindowPos(ImVec2(position - twidth / 2, screen.y * .33f + headerSize));
		ImGui::BeginChild("Highscores table", ImVec2(twidth, theight));
		if (ImGui::BeginTable("Upgrades", 3))
		{
			ImGui::TableSetupColumn("#",     ImGuiTableColumnFlags_WidthStretch, .1);
			ImGui::TableSetupColumn("Name",  ImGuiTableColumnFlags_WidthStretch);
			ImGui::TableSetupColumn("Score", ImGuiTableColumnFlags_WidthStretch, .5);

			for (int i = 0; i < records.size(); i++)
			{
				HighscoreRecord& record = records.at(i);

				ImGui::TableNextColumn();
				ImGui::Text("%d", record.order);

				ImGui::TableNextColumn();
				ImGui::Text(record.name.c_str());

				ImGui::TableNextColumn();
				ImGui::Text("%d", record.score);

				ImGui::TableNextRow();
			}
			ImGui::EndTable();
		}

		ImGui::PopFont();
		ImGui::EndChild();
	}

	struct SettingsState
	{
		bool fullscreen;
	};

	SettingsState state;

	void Settings(float position)
	{
		auto [width, height] = GetSize(title_settings, 100);

		ImGui::SetCursorPos(ImVec2(position - screen.x / 2 + margin * 2, margin * 2));
		ImGui::Image((void*)title_settings->DeviceHandle(), ImVec2(width, height));

		ImGui::SetCursorPos(ImVec2(position - screen.x / 2.f + 2 * margin, midpoint.y));
		if (ImGui::Button("Back"))
		{
			PressedBack();
		}

		ImGui::PushFont(FontMap::Get("Highscore Table"));

		float twidth  = screen.x * .5f;
		float theight = screen.y * .5f;

		ImGui::SetNextWindowPos(ImVec2(position - twidth / 2, screen.y * .33f));
		ImGui::BeginChild("Settings Items", ImVec2(twidth, -1));

		ImGui::Text("Video");
		ImGui::Separator();
		ImGui::Checkbox("Fullscreen", &state.fullscreen);

		ImGui::EndChild();

		ImGui::PopFont();
	}

	bool MenuButton(const char* label, MenuState state)
	{
		ImVec2 textSize = ImGui::CalcTextSize(label);
		ImGui::SetCursorPosX(midpoint.x - textSize.x / 2);

		bool active = false;

		if (ImGui::Button(label))
		{
			active = true;
		}
		PutDotsNextToActiveButton(state);

		return active;
	}

	void PutDotsNextToActiveButton(MenuState state)
	{
		if (ImGui::IsItemHovered() || state == currentState)
		{
			currentState = state;

			ImVec2 min = ImGui::GetItemRectMin();
			ImVec2 max = ImGui::GetItemRectMax();
			float avgY = (min.y + max.y) / 2.f;

			ImVec2 dotLeft  = ImVec2(min.x - 2 * margin, avgY);
			ImVec2 dotRight = ImVec2(max.x + 2 * margin, avgY);

			ImGui::GetForegroundDrawList()->AddCircleFilled(dotLeft,  5.f, Color(255, 255, 255).as_u32);
			ImGui::GetForegroundDrawList()->AddCircleFilled(dotRight, 5.f, Color(255, 255, 255).as_u32);
		}
	}
};