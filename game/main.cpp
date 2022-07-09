#include "EngineLoop.h"
#include "Common.h"
#include "Rendering.h"

#include "GameRender.h"

#include "Sand/Sand.h"
#include "Sand/SandEntity.h"

#include "ext/Time.h"
#include "ext/MeshGenerators.h"
#include "ext/Components.h"

#include "imgui/implot.h"

// systems

#include "Sand/SandSystems.h"
#include "Systems/PlayerController.h"
#include "Systems/FlockingMovement.h"
#include "Systems/TurnTwoardsTarget.h"
#include "Systems/ExplodeNearTarget.h"
#include "Systems/ExplosionSpawner.h"
#include "Systems/EnemyController.h"
#include "Systems/KeepOnScreen.h"
#include "Systems/FireWeaponAfterDelay.h"
#include "Systems/FireWeapon.h"
#include "Systems/RockSpawner_Test.h"
#include "Systems/ItemSystem.h"
#include "Systems/ItemSpawner.h"
#include "Systems/ItemPickup.h"
#include "Systems/RenderSceneGraph.h"
#include "Systems/LowCorePixelDeath.h"
#include "Systems/zTestingSystem.h"
#include "Systems/EnemySpawner.h"

#include "UI/PlayerHUD.h"

#include "ext/systems/PhysicsInterpolation.h"
#include "ext/systems/ParticleUpdate.h"
#include "ext/systems/DestroyInTime.h"

struct MetricsSystem : System<MetricsSystem>
{
	std::vector<float> m_time;

	std::vector<float> m_fixedTicks;
	std::vector<float> m_deltaTime;
	std::vector<int> m_cellCounts;

	void Init()
	{
		//Attach<event_RecordMetric>();
	}

	void UI()
	{
		ImGui::Begin("Metrics");
		ImGui::Text("Delta time: %f", Time::RawDeltaTime());
		ImGui::Text("Scaled Delta time: %f", Time::DeltaTime());

		float timeScale = Time::TimeScale();
		ImGui::SliderFloat("time scale", &timeScale, 0, 2);
		Time::SetTimeScale(timeScale);

		//for (auto [transform, flocker] : Query<Transform2D, Flocker>())
		//{
		//	ImGui::Text("x: %f y: %f", transform.position.x, transform.position.y);
		//}

		//int cellCount = 0;

		//for (auto _ : Query<Cell>())
		//{
		//	cellCount += 1;
		//}

		//m_cellCounts.push_back(cellCount);

		//ImPlot::SetNextAxesToFit();
		//if (ImPlot::BeginPlot("Times"))
		//{
		//	ImPlot::PlotLine("Delta time", m_time.data(), m_deltaTime .data(), m_time.size());
		//	ImPlot::PlotLine("Fixed time", m_time.data(), m_fixedTicks.data(), m_time.size());
		//	ImPlot::EndPlot();
		//}

		ImGui::End();

		if (m_time.size() > 10.f / Time::DeltaTime())
		{
			m_time      .erase(m_time      .begin());
			m_deltaTime .erase(m_deltaTime .begin());
			m_fixedTicks.erase(m_fixedTicks.begin());
		}
	}

	void on(event_RecordMetric& e)
	{
		switch (e.metric)
		{
			case TICK:       
			{
				m_time     .push_back(e.timePoint);
				m_deltaTime.push_back(e.value);
				break;
			}
			case TICK_FIXED: 
			{
				m_fixedTicks.push_back(e.value);
				break;
			}
		}
	}
};

struct Regolith : EngineLoop
{
	void _Init()
	{
		ConfigureWindow();
		InitGameRenderVars();

		ConfigureModules();
		ConfigureLevel();
		ConfigureInputMapping();
		ConfigureMainGameLevel();
	}

	// Init

	void ConfigureWindow()
	{
		Window& window = m_app.GetModule<Window>();

		window.Resize(1280, 720);
		window.SetTitle("Windowing Test");
	}

	void ConfigureLevel()
	{
		r<Level> level = LevelManager::CurrentLevel();

		level->AddSystem(System_RenderSceneGraph());

		level->AddSystem(System_PhysicsInterpolation());
		level->AddSystem(System_DestroyInTime());

		level->AddSystem(System_PlayerController());
		level->AddSystem(System_TurnTwoardsTarget());
		level->AddSystem(System_FlockingMovement());
		level->AddSystem(System_ExplodeNearTarget());
		level->AddSystem(System_ExplosionSpawner());
		level->AddSystem(System_EnemyController());
		level->AddSystem(System_KeepOnScreen());
		level->AddSystem(System_FireWeaponAfterDelay());
		level->AddSystem(System_FireWeapon());
		level->AddSystem(System_ParticleUpdate());
		level->AddSystem(System_Item());
		level->AddSystem(System_ItemSpawner());
		level->AddSystem(System_ItemPickup());
		level->AddSystem(System_LowCorePixelDeath());
		level->AddSystem(System_EnemySpawner());

		level->AddSystem(System_UI_PlayerHUD());

		level->AddSystem(MetricsSystem());
		
		//level->AddSystem(System_Testing());
		//level->AddSystem(System_RockSpawner_Test());

		AddSandSystemsToLevel(level);
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

	// this should load from a file that the user can configure in a settings menu...

	void ConfigureInputMapping()
	{
		InputMapping& input = m_app.GetModule<Window>().Input();

		input.m_keyboard[SDL_SCANCODE_W] = InputName::UP;
		input.m_keyboard[SDL_SCANCODE_S] = InputName::DOWN;
		input.m_keyboard[SDL_SCANCODE_D] = InputName::RIGHT;
		input.m_keyboard[SDL_SCANCODE_A] = InputName::LEFT;
	}

	void ConfigureMainGameLevel()
	{ 
		r<Level> level = LevelManager::CurrentLevel();

		Entity player = CreateSandSprite("player.png", "player_collider_mask.png");
		player.Add<Player>();
		player.Add<KeepOnScreen>();
		player.Add<ItemSink>();
		player.Add<SandHealable>();
		player.Add<SandDieInTimeWithLowCoreCount>();

		player.Add<Rigidbody2D>().SetFixedRotation(true);
		player.Get<SandSprite>().isHardCore = true;

		//Entity e = CreateSandSprite("test_sqr.png", "test_sqr.png");
		//e.Get<Transform2D>().rotation = wPI / 6.f;
	}
};

void setup()
{
	RunEngineLoop<Regolith>();
}
