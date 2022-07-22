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


#include "ext/systems/PhysicsInterpolation.h"
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
	void _Init()
	{
		ConfigureWindow();
		InitGameRenderVars();

		ConfigureModules();
		ConfigureLevel();
		ConfigureInputMapping();
		//ConfigureMainGameLevel();

		m_app.Attach<event_SubmitHighscore>(this);
		m_app.Attach<event_PlayGame>(this);
	}

	void _InitUI()
	{
		FontMap& fonts = m_app.GetModule<FontMap>();
		fonts.Load("Roboto",          18,     "Roboto.ttf");
		fonts.Load("Score",           56 * 2, "Roboto.ttf");
		fonts.Load("OK Button",       32 * 2, "Roboto.ttf");
		fonts.Load("Game Over",      128 * 2, "Roboto.ttf");
		fonts.Load("Final Score",     64 * 2, "Roboto.ttf");
		fonts.Load("Highscore Input", 64 * 2, "Roboto.ttf");

		fonts.Load("Main Menu Title", 192 * 2, "Roboto.ttf");
		fonts.Load("Main Menu Button", 64 * 2, "Roboto.ttf");
	}

	// this should load from a file that the user can configure in a settings menu...

	void ConfigureInputMapping()
	{
		InputMap& inputs = m_app.GetModule<InputMap>();

		inputs.Set(SDL_SCANCODE_UP,     InputName::UP);
		inputs.Set(SDL_SCANCODE_DOWN,   InputName::DOWN);
		inputs.Set(SDL_SCANCODE_RIGHT,  InputName::RIGHT);
		inputs.Set(SDL_SCANCODE_LEFT,   InputName::LEFT);
		inputs.Set(SDL_SCANCODE_SPACE,  InputName::ATTACK);
		inputs.Set(SDL_SCANCODE_ESCAPE, InputName::ESCAPE);
	}

	// Init

	void ConfigureWindow()
	{
		Window& window = m_app.GetModule<Window>();

		window.Resize(1280, 720);
		window.SetTitle("Windowing Test");
	}

	std::vector<SystemBase*> thegame;
	std::vector<SystemBase*> themenu;

	void ConfigureLevel()
	{
		r<Level> level = LevelManager::CurrentLevel();

		// Basic functionality
		level->CreateSystem(System_RenderScene());
		level->CreateSystem(System_PhysicsInterpolation());
		level->CreateSystem(System_ParticleUpdate());
		level->CreateSystem(System_DestroyInTime());
		level->CreateSystem(System_KeepOnScreen());
		CreateSandSystems(level);

		SwitchToMenu();
	}

	void SwitchToGame()
	{
		r<Level> level = LevelManager::CurrentLevel();

		m_app.GetModule<Camera>().x = 0;
		m_app.GetModule<Camera>().y = 0;

		level->GetWorld()->Clear();
		// doesnt clear the physics world

		thegame = {
			// Gameplay functionality
			level->CreateSystem(System_PlayerSpawner()),
			level->CreateSystem(System_PlayerController()),
			level->CreateSystem(System_LowCorePixelDeath()),
			level->CreateSystem(System_Item()),
			level->CreateSystem(System_ItemPickup()),
			level->CreateSystem(System_FireWeapon()),
			level->CreateSystem(System_RockSpawner_Test()),
			
			// UI
			level->CreateSystem(System_UI_AsteroidsHUD())
		};

		level->DestroySystems(themenu);
		themenu = {};
	}

	void SwitchToMenu()
	{
		r<Level> level = LevelManager::CurrentLevel();

		themenu = {
			level->CreateSystem(System_UI_AsteroidsMenu())
		};

		level->DestroySystems(thegame);
		thegame = {};
	}

	void ConfigureModules()
	{
		Window& window = m_app.GetModule<Window>();

		vec2 screenSize = vec2(window.Width(), window.Height());
		vec2 cameraSize = vec2(16 * 2, 9 * 2);

		int cellsPerMeter = 18;

		m_app.AddModule<SandWorld>(cellsPerMeter, vec2(cameraSize.x, cameraSize.y));
		m_app.AddModule<Camera>(0, 0, cameraSize.x, cameraSize.y);          // this is bad but works ok for now...

		CoordTranslation coords;
		coords.ScreenToWorld = cameraSize;
		coords.CellsToMeters = 1.f / cellsPerMeter;

		m_app.AddModule<CoordTranslation>(coords);
	}

	float transitionTimer = 0;
	float transitionTime = 2.f;

	float easeQuint(float x)
	{
		return x * x * x * x;
	}

	void on(event_SubmitHighscore& e)
	{
		m_app.GetTaskPool()->Coroutine([this]()
		{
			transitionTimer += Time::DeltaTime();
			
			float progress = transitionTimer / transitionTime;
			bool done = progress > 1.f;
			
			if (done)
			{
				SwitchToMenu();
			}

			Camera& camera = m_app.GetModule<Camera>();
			vec2 position = vec2(camera.x, camera.y);

			position = lerp(position, vec2(100, 30), easeQuint(progress));

			camera.x = position.x;
			camera.y = position.y;

			return done;
		});
	}

	void on(event_PlayGame& e)
	{
		SwitchToGame();
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