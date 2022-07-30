#include "Common.h"
#include "Rendering.h"

#include "Data.h"
#include "GameRender.h"

#include "Sand/Sand.h"
#include "Sand/SandEntity.h"

#include "app/EngineLoop.h"
#include "ext/MeshGenerators.h"

#include "imgui/implot.h"

// systems

#include "Sand/SandSystems.h"
#include "Systems/InitGame.h"
#include "Systems/AllowPauseMenu.h"
#include "Systems/PlayerController.h"
#include "Systems/PlayerSpawner.h"
#include "Systems/KeepOnScreen.h"
#include "Systems/FireWeapon.h"
#include "Systems/RockSpawner_Test.h"
#include "Systems/ItemSystem.h"
#include "Systems/ItemPickup.h"
#include "Systems/RenderScene.h"
#include "Systems/LowCorePixelDeath.h"
#include "Systems/zTestingSystem.h"
#include "Systems/Background.h"
#include "Systems/HighscoreStore.h"

#include "Systems/EnemyController.h"
#include "Systems/EnemySpawner.h"
#include "Systems/EnemySystem.h"
#include "Systems/FlockingMovement.h"
#include "Systems/FireWeaponAfterDelay.h"

#include "Systems/LightningSystem.h"

#include "UI/AsteroidsHUD.h"
#include "UI/AsteroidsMenu.h"

#include "ext/systems/ParticleUpdate.h"
#include "ext/systems/DestroyInTime.h"
#include "ext/systems/MetricsSystem.h"
#include "ext/systems/ConsoleSystem.h"

// Idea: Make a polished version of asteroids to completion, then iterate with enemies and damage system

struct Regolith : EngineLoop<Regolith>
{
	void Init()
	{
		ConfigureWindow();
		ConfigureAudio();
		InitGameRenderVars();

		ConfigureLevel();
		ConfigureInputMapping();

		ConfigureConsole();

		Attach<event_SubmitHighscore>();
		Attach<event_PlayGame>();

		CreateDBAndTablesIfNeeded();
	}

	void Dnit()
	{
		ClearPrefabs();
	}

	void InitUI()
	{
		FontMap::Load("Roboto",           18,     "Roboto.ttf");
		FontMap::Load("Score",            56 * 2, "Roboto.ttf");
		FontMap::Load("OK Button",        32 * 2, "Roboto.ttf");
		FontMap::Load("Game Over",       128 * 2, "Roboto.ttf");
		FontMap::Load("Final Score",      64 * 2, "Roboto.ttf");
		FontMap::Load("Highscore Input",  64 * 2, "Roboto.ttf");
		FontMap::Load("Main Menu Title", 192 * 2, "Roboto.ttf");
		FontMap::Load("Main Menu Button", 48 * 2, "DidactGothic-Regular.ttf");
		FontMap::Load("Highscore Table",  36 * 2, "DidactGothic-Regular.ttf");

		ImGuiStyle& style = ImGui::GetStyle();
		style.WindowPadding                         = ImVec2(0, 0);
		style.FramePadding                          = ImVec2(0.f, 0.f);
		style.WindowBorderSize                      = 0.f;
		style.Colors[ImGuiCol_Text]                 = ImVec4(1, 1, 1, 1);
		style.Colors[ImGuiCol_Button]               = ImVec4(0, 0, 0, 0);
		style.Colors[ImGuiCol_ButtonActive]         = ImVec4(0, 0, 0, 0);
		style.Colors[ImGuiCol_ButtonHovered]        = ImVec4(0, 0, 0, 0);
		style.Colors[ImGuiCol_TableRowBg]           = ImVec4(1, 1, 1, 1);
		style.Colors[ImGuiCol_ScrollbarBg]          = ImVec4(0, 0, 0, 0);
		style.Colors[ImGuiCol_ScrollbarGrab]        = ImVec4(1, 1, 1, .2);
		style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(1, 1, 1, .6);
		style.Colors[ImGuiCol_ScrollbarGrabActive]  = ImVec4(1, 1, 1, .4);
		style.Colors[ImGuiCol_CheckMark]            = ImVec4(1, 1, 1, .9);
		style.Colors[ImGuiCol_FrameBg]              = ImVec4(1, 1, 1, .2);
		style.Colors[ImGuiCol_FrameBgHovered]       = ImVec4(1, 1, 1, .4);
		style.Colors[ImGuiCol_FrameBgActive]        = ImVec4(1, 1, 1, .2);
		style.Colors[ImGuiCol_SliderGrab]           = ImVec4(1, 1, 1, .5);
		style.Colors[ImGuiCol_SliderGrabActive]     = ImVec4(1, 1, 1, .8);

		style.Colors[ImGuiCol_Header]               = ImVec4(1, 1, 1, .2);
		style.Colors[ImGuiCol_HeaderActive]         = ImVec4(1, 1, 1, .3);
		style.Colors[ImGuiCol_HeaderHovered]        = ImVec4(1, 1, 1, .5);

		//ImGuiIO& io = ImGui::GetIO();
		//io.NavMovesMouse = true;
	}

	// this should load from a file that the user can configure in a settings menu...

	void ConfigureInputMapping()
	{
		Input::SetMap({
			{ SDL_SCANCODE_UP,     InputName::UP},
			{ SDL_SCANCODE_DOWN,   InputName::DOWN},
			{ SDL_SCANCODE_RIGHT,  InputName::RIGHT},
			{ SDL_SCANCODE_LEFT,   InputName::LEFT},
			{ SDL_SCANCODE_SPACE,  InputName::ATTACK},
			{ SDL_SCANCODE_ESCAPE, InputName::ESCAPE}
		});
	}

	// Init

	void ConfigureWindow()
	{
		Window& window = app.GetWindow();

		window.Resize(1280, 720);
		window.SetTitle("Regolith");
	}

	void ConfigureAudio()
	{
		Audio& audio = app.GetAudio();
		audio.Load("Sounds/Master.strings.bank");
		audio.Load("Sounds/Master.bank");
	}

	void ConfigureLevel()
	{
		CreateMenuWorld();
		CreateBackgroundWorld();
	}

	World* menu;
	World* game;

	void CreateMenuWorld()
	{
		menu = app.CreateWorld();
		menu->CreateSystem<System_UI_AsteroidsMenu>();
	}

	void CreateGameWorld()
	{
		game = app.CreateWorld();

		game->CreateSystem<InitGame_System>();

		// Basic functionality

		game->CreateSystem<System_Render_Meshes>();
		game->CreateSystem<System_Render_Sprites>();
		game->CreateSystem<System_Render_SandCollisionInfo>();
		game->CreateSystem<System_ParticleUpdate>();
		game->CreateSystem<System_DestroyInTime>();
		game->CreateSystem<System_KeepOnScreen>();
		
		CreateSandSystems(game);

		// Gameplay functionality
		game->CreateSystem<System_PlayerSpawner>();
		game->CreateSystem<System_PlayerController>();
		game->CreateSystem<System_LowCorePixelDeath>();
		game->CreateSystem<System_Item>();
		game->CreateSystem<System_ItemPickup>();
		game->CreateSystem<System_FireWeapon>();
		game->CreateSystem<System_RockSpawner_Test>();
		game->CreateSystem<System_Lightning>();

		// enemy
		game->CreateSystem<System_EnemySpawner>();
		game->CreateSystem<System_EnemyController>();
		game->CreateSystem<System_Enemy>();
		game->CreateSystem<System_FlockingMovement>();
		game->CreateSystem<System_FireWeaponAfterDelay>();

		// UI
		game->CreateSystem<System_UI_AsteroidsHUD>();
	}

	void CreateBackgroundWorld()
	{
		World* bg = app.CreateWorld();
		bg->CreateSystem<System_Render_Sprites>();
		bg->CreateSystem<Background_System>();
		bg->CreateSystem<System_HighscoreStore>();
		bg->CreateSystem<System_Console>();
	}

	void ConfigureConsole()
	{
		app.GetConsole().RegCommand("play", [this](const ConsoleCommand& cmd) 
		{
			app.GetAudio().Play(cmd.GetString(0));
		});

		app.GetConsole().RegCommand("stop", [this](const ConsoleCommand& cmd) 
		{
			int handle = 0;
			if (cmd.Is(0, INT)) handle = cmd.GetInt(0);
			else handle = app.GetAudio().GetHandle(cmd.GetString(0));
			app.GetAudio().Stop(handle);
		});

		app.GetConsole().RegCommand("free", [this](const ConsoleCommand& cmd) 
		{
			int handle = 0;
			if (cmd.Is(0, INT)) handle = cmd.GetInt(0);
			else handle = app.GetAudio().GetHandle(cmd.GetString(0));
			app.GetAudio().Free(handle);
		});

		app.GetConsole().RegCommand("list_audio", [this](const ConsoleCommand& cmd) 
		{
			log_audio("Audio instances:");
			for (auto [name, inst] : app.GetAudio().m_loaded)
			{
				log_audio("\t%d. %s", inst, name.c_str());
			}
		});

		app.GetConsole().RegCommand("set_time_scale", [this](const ConsoleCommand& cmd)
		{
			Time::SetTimeScale(cmd.GetFloat(0));
		});

		app.GetConsole().RegCommand("set_fixed_step", [this](const ConsoleCommand& cmd)
		{
			Time::SetFixedTime(cmd.GetFloat(0));
		});
	}

	float transitionTimer = 0;
	float transitionTime = 2.f;

	float easeQuint(float x)
	{
		return x * x * x * x;
	}

	void on(event_PlayGame& e)
	{
		app.DestroyWorld(menu);
		menu = nullptr;
		CreateGameWorld();
	}

	void on(event_SubmitHighscore& e)
	{
		app.DestroyWorld(game);
		game = nullptr;
		CreateMenuWorld();
	}
};

int main()
{
	RunEngineLoop<Regolith>();
	return 0;
}