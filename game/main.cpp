#include "Common.h"
#include "Rendering.h"

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
#include "UI/AsteroidsHUD.h"
#include "UI/AsteroidsMenu.h"

#include "ext/systems/ParticleUpdate.h"
#include "ext/systems/DestroyInTime.h"
#include "ext/systems/MetricsSystem.h"

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

		Attach<event_SubmitHighscore>();
		Attach<event_PlayGame>();
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

		// enemy
		// game->CreateSystem<EnemyController_System>();

		// UI
		game->CreateSystem<System_UI_AsteroidsHUD>();
	}

	void CreateBackgroundWorld()
	{
		World* bg = app.CreateWorld();
		bg->CreateSystem<System_Render_Sprites>();
		bg->CreateSystem<Background_System>();
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

void setup()
{
	RunEngineLoop<Regolith>();
}