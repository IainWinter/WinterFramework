#include "EngineLoop.h"
#include "Common.h"
#include "Rendering.h"

#include "Sand/Sand.h"
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

#include "ext/systems/PhysicsInterpolation.h"
#include "ext/systems/ParticleUpdate.h"
#include "ext/systems/DestroyInTime.h"

#include "Systems/RenderSceneGraph.h"

#include "Systems/zTestingSystem.h"

#include "GameRender.h"

struct MetricsSystem : System<MetricsSystem>
{
	std::vector<float> m_time;

	std::vector<float> m_fixedTicks;
	std::vector<float> m_deltaTime;
	std::vector<int> m_cellCounts;

	void Init()
	{
		Attach<event_RecordMetric>();
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

		ImPlot::SetNextAxesToFit();
		if (ImPlot::BeginPlot("Times"))
		{
			ImPlot::PlotLine("Delta time", m_time.data(), m_deltaTime .data(), m_time.size());
			ImPlot::PlotLine("Fixed time", m_time.data(), m_fixedTicks.data(), m_time.size());
			ImPlot::EndPlot();
		}

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

		level->AddSystem(MetricsSystem());
		level->AddSystem(System_Testing());

		//level->AddSystem(System_RockSpawner_Test());

		AddSandSystemsToLevel(level);
	}

	void ConfigureModules()
	{
		Window& window = m_app.GetModule<Window>();

		vec2 screenSize = vec2(window.Width(), window.Height());
		vec2 cameraSize = vec2(16 * 2, 9 * 2);

		m_app.AddModule<SandWorld>(20, vec2(cameraSize.x, cameraSize.y));
		m_app.AddModule<Camera>(0, 0, cameraSize.x, cameraSize.y);          // this is bad but works ok for now...

		CoordTranslation coords;
		coords.ScreenToWorld = cameraSize;

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
		player.Add<Rigidbody2D>().SetFixedRotation(true);
		player.Add<KeepOnScreen>();
		player.Add<ItemSink>();
		player.Add<SandHealable>();

		player.Get<SandSprite>().invulnerable = true;

		Entity e = CreateSandSprite("test_line.png", "test_line.png");
		e.Get<Transform2D>().rotation = wPI / 6.f;

		m_app.GetRootEventQueue()->send(event_Sand_ExplodeToDust{ e });

		if (false)
		for (int i = 0; i < 10; i++)
		{
			Entity entity;

			switch (get_rand(2))
			{
				case 0: // fighter
				{
					entity = CreateSandSprite("enemy_fighter.png", "enemy_fighter_mask.png");
					entity.Add<FireWeaponAfterDelay>(player, WEAPON_LASER, 1.f + get_randc(.3f));
					entity.Add<TurnTwoardsTarget>(level->CreateEntity().AddAll(Transform2D(get_randc(32), get_randc(18))));
					entity.Add<Flocker>();

					break;
				}

				case 1: // bomb
				{
					entity = CreateSandSprite("enemy_bomb.png", "enemy_bomb_mask.png");
					entity.Add<TurnTwoardsTarget>(player);

					break;
				}

				case 2: // station
				{
					entity = CreateSandSprite("enemy_station.png", "enemy_station_mask.png");
					entity.Add<TurnTwoardsTarget>(level->CreateEntity().AddAll(Transform2D(get_rand(32, 18))));
					entity.Add<Flocker>();

					break;
				}
			}

			Transform2D& transform = entity.Get<Transform2D>();
			transform.position = get_randc(20.f, 20.f);

			entity.Add<Rigidbody2D>(transform).SetFixedRotation(true);
		}
	}

	Entity CreateSandSprite(const std::string& path, const std::string& collider_mask_path)
	{
		auto [sand, physics] = m_app.GetModules<SandWorld, PhysicsWorld>();

		r<Texture> sprite = mkr<Texture>(_a(path), false);
		r<Texture> mask   = mkr<Texture>(_a(collider_mask_path), false);

		assert(sprite->Length() == mask->Length());

		Transform2D transform;

		Entity entity = LevelManager::CurrentLevel()->CreateEntity();
		entity.Add<Transform2D>(transform);
		entity.Add<SandSprite>(mask);
		entity.Add<Sprite>(sprite);

		m_app.GetRootEventQueue()->send(event_SandAddSprite { entity });

		return entity;
	}

	Entity CreateTexturedCircle(const std::string& path, const std::string& path_mask)
	{
		auto [sand, physics] = m_app.GetModules<SandWorld, PhysicsWorld>();

		r<Texture> sprite = mkr<Texture>(_a(path), false);
		r<Texture> mask   = mkr<Texture>(_a(path_mask), false);
		
		assert(sprite->Length() == mask->Length());

		Transform2D transform;
		//transform.scale = vec2(sprite->Width(), sprite->Height()) * sand.worldScale;

		Entity entity = LevelManager::CurrentLevel()->CreateEntity();
		entity.Add<Transform2D>(transform);
		entity.Add<SandSprite>(mask);
		entity.Add<Sprite>(sprite);

		Rigidbody2D& body = physics.AddEntity(entity);

		b2CircleShape shape;
		shape.m_radius = max(transform.scale);
		
		b2Fixture* collider = body.AddCollider(shape, 100.f);
		collider->SetRestitution(.5f);

		m_app.GetRootEventQueue()->send(event_SandAddSprite{ entity });

		return entity;
	}
};

void setup()
{
	RunEngineLoop<Regolith>();
}
