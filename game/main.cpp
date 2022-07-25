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
#include "Systems/InitMainMenu.h"
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

//#include "Systems/FlockingMovement.h"
//#include "Systems/TurnTwoardsTarget.h"
//#include "Systems/ExplodeNearTarget.h"
//#include "Systems/ExplosionSpawner.h"
//#include "Systems/EnemyController.h"
//#include "Systems/EnemySpawner.h"
//#include "Systems/EnemySystem.h"
//#include "Systems/FireWeaponAfterDelay.h"
//#include "Systems/ItemSpawner.h"
//#include "Systems/LightningSystem.h"
//#include "Systems/TankSystem.h"
//#include "UI/PlayerHUD.h"


#include "ext/systems/ParticleUpdate.h"
#include "ext/systems/DestroyInTime.h"
#include "ext/systems/MetricsSystem.h"

//struct MetricsSystem : System<MetricsSystem>
//{
//	std::vector<float> m_time;
//
//	std::vector<float> m_fixedTicks;
//	std::vector<float> m_deltaTime;
//	std::vector<int> m_cellCounts;
//
//	void Init()
//	{
//		//Attach<event_RecordMetric>();
//	}
//
//	void UI()
//	{
//		ImGui::Begin("Metrics");
//		ImGui::Text("Delta time: %f", Time::RawDeltaTime());
//		ImGui::Text("Scaled Delta time: %f", Time::DeltaTime());
//
//		float timeScale = Time::TimeScale();
//		ImGui::SliderFloat("time scale", &timeScale, 0, 2);
//		Time::SetTimeScale(timeScale);
//
//		//for (auto [transform, flocker] : Query<Transform2D, Flocker>())
//		//{
//		//	ImGui::Text("x: %f y: %f", transform.position.x, transform.position.y);
//		//}
//
//		//int cellCount = 0;
//
//		//for (auto _ : Query<Cell>())
//		//{
//		//	cellCount += 1;
//		//}
//
//		//m_cellCounts.push_back(cellCount);
//
//		//ImPlot::SetNextAxesToFit();
//		//if (ImPlot::BeginPlot("Times"))
//		//{
//		//	ImPlot::PlotLine("Delta time", m_time.data(), m_deltaTime .data(), m_time.size());
//		//	ImPlot::PlotLine("Fixed time", m_time.data(), m_fixedTicks.data(), m_time.size());
//		//	ImPlot::EndPlot();
//		//}
//
//		ImGui::End();
//
//		if (m_time.size() > 10.f / Time::DeltaTime())
//		{
//			m_time      .erase(m_time      .begin());
//			m_deltaTime .erase(m_deltaTime .begin());
//			m_fixedTicks.erase(m_fixedTicks.begin());
//		}
//	}
//
//	void on(event_RecordMetric& e)
//	{
//		switch (e.metric)
//		{
//			case TICK:       
//			{
//				m_time     .push_back(e.timePoint);
//				m_deltaTime.push_back(e.value);
//				break;
//			}
//			case TICK_FIXED: 
//			{
//				m_fixedTicks.push_back(e.value);
//				break;
//			}
//		}
//	}
//};

// Idea: Make a polished version of asteroids to completion, then iterate with enemies and damage system

struct Regolith : EngineLoop
{
	void Init()
	{
		ConfigureWindow();
		InitGameRenderVars();

		ConfigureLevel();
		ConfigureInputMapping();

		app.GetRootEventBus().Attach<event_SubmitHighscore>(this);
		app.GetRootEventBus().Attach<event_PlayGame>(this);
	}

	void InitUI()
	{
		FontMap::Load("Roboto",          18,     "Roboto.ttf");
		FontMap::Load("Score",           56 * 2, "Roboto.ttf");
		FontMap::Load("OK Button",       32 * 2, "Roboto.ttf");
		FontMap::Load("Game Over",      128 * 2, "Roboto.ttf");
		FontMap::Load("Final Score",     64 * 2, "Roboto.ttf");
		FontMap::Load("Highscore Input", 64 * 2, "Roboto.ttf");
		FontMap::Load("Main Menu Title", 192 * 2, "Roboto.ttf");
		FontMap::Load("Main Menu Button", 64 * 2, "Roboto.ttf");
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

		menu->CreateSystem(InitMainMenu_System());
		menu->CreateSystem(System_UI_AsteroidsMenu());
	}

	void CreateGameWorld()
	{
		game = app.CreateWorld();

		game->CreateSystem(InitGame_System());

		// Basic functionality

		game->CreateSystem(System_Render_Meshes());
		game->CreateSystem(System_Render_Sprites());
		game->CreateSystem(System_Render_SandCollisionInfo());
		game->CreateSystem(System_ParticleUpdate());
		game->CreateSystem(System_DestroyInTime());
		game->CreateSystem(System_KeepOnScreen());
		
		CreateSandSystems(game);

		// Gameplay functionality
		game->CreateSystem(System_PlayerSpawner());
		game->CreateSystem(System_PlayerController());
		game->CreateSystem(System_LowCorePixelDeath());
		game->CreateSystem(System_Item());
		game->CreateSystem(System_ItemPickup());
		game->CreateSystem(System_FireWeapon());
		game->CreateSystem(System_RockSpawner_Test());

		// UI
		game->CreateSystem(System_UI_AsteroidsHUD());
	}

	void CreateBackgroundWorld()
	{
		World* bg = app.CreateWorld();
		bg->CreateSystem(System_Render_Sprites());
		bg->CreateSystem(Background_System());
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


//level->AddSystem(System_ItemSpawner()),
//level->AddSystem(System_FireWeaponAfterDelay()),
//level->AddSystem(System_Enemy()),
//level->AddSystem(System_ExplodeNearTarget()),
//level->AddSystem(System_AllowPauseMenu()),
//level->AddSystem(System_FuelTank()),
//level->AddSystem(System_EnemySpawner()),
//level->AddSystem(System_TurnTwoardsTarget());
//level->AddSystem(System_EnemyController());
//level->AddSystem(System_FlockingMovement());
//level->AddSystem(System_ExplosionSpawner());
//level->AddSystem(System_Lightning());
//level->AddSystem(System_UI_PlayerHUD());
//level->AddSystem(System_Metrics());
//level->AddSystem(System_Testing());