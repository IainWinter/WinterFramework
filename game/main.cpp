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

#include "Systems//PlayerController.h"
#include "Systems/FlockingMovement.h"
#include "Systems/TurnTwoardsTarget.h"
#include "Systems/ExplodeNearTarget.h"
#include "Systems/ExplosionSpawner.h"
#include "Systems/EnemyController.h"
#include "Systems/KeepOnScreen.h"

#include "Systems/FireWeaponAfterDelay.h"

#include "ext/systems/PhysicsInterpolation.h"
#include "ext/systems/SimpleSpriteRender.h"
#include "ext/systems/SimpleMeshRender.h"

//struct SandSpriteMaskRenderer : SystemBase
//{
//	SpriteRenderer2D render;
//
//	void Update() override
//	{
//		auto [camera] = GetModules<Camera>();
//
//		render.Begin(camera);
//		for (auto [transform, sprite] : Query<Transform2D, SandSprite>())
//		{
//			render.m_shader.Set("tint", vec4(1, 0, 0, 1));
//			render.DrawSprite(transform, sprite.Get());
//		}
//	}
//};

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

		for (auto [transform, flocker] : Query<Transform2D, Flocker>())
		{
			ImGui::Text("x: %f y: %f", transform.position.x, transform.position.y);
		}

		int cellCount = 0;

		for (auto _ : Query<Cell>())
		{
			cellCount += 1;
		}

		m_cellCounts.push_back(cellCount);

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
				m_fixedTicks.push_back(e.value * Time::DeltaTime());
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

		level->AddSystem(PhysicsInterpolation());
		level->AddSystem(SimpleSpriteRenderer2D());
		level->AddSystem(SimpleMeshRenderer2D());

		level->AddSystem(System_PlayerController());
		level->AddSystem(System_TurnTwoardsTarget());
		level->AddSystem(System_FlockingMovement());
		level->AddSystem(System_ExplodeNearTarget());
		level->AddSystem(System_ExplosionSpawner());
		level->AddSystem(System_EnemyController());
		level->AddSystem(System_KeepOnScreen());
		level->AddSystem(System_FireWeaponAfterDelay());

		level->AddSystem(MetricsSystem());

		AddSandSystemsToLevel(level);
	}

	void ConfigureModules()
	{
		m_app.AddModule<SandWorld>(1280, 720, 32, 18);
		m_app.AddModule<Camera>(0, 0, 32, 18);          // this is bad but works ok for now...

		CoordTranslation coords;
		coords.ScreenToWorld = vec2(32, 18);

		m_app.AddModule<CoordTranslation>(coords);
	}

	// this should load from a file that the user can configure in a settings menu...

	void ConfigureInputMapping()
	{
		InputMapping& input = m_app.GetModule<Window>().m_input;

		input.m_keyboard[SDL_SCANCODE_W] = InputName::UP;
		input.m_keyboard[SDL_SCANCODE_S] = InputName::DOWN;
		input.m_keyboard[SDL_SCANCODE_D] = InputName::RIGHT;
		input.m_keyboard[SDL_SCANCODE_A] = InputName::LEFT;
	}

	void ConfigureMainGameLevel()
	{
		r<Level> level = LevelManager::CurrentLevel();

		Entity player = CreateSandSprite("player.png", "player_collider_mask.png");
		player.Get<SandSprite>().invulnerable = true;
		player.Add<Player>();
		player.Add<Rigidbody2D>().SetFixedRotation(true);
		player.Add<KeepOnScreen>();

		Entity target = level->CreateEntity().AddAll(Transform2D(vec2(0.f, 0.f)));

		Entity entity = CreateSandSprite("test_sqr.png", "test_sqr.png");
		entity.Add<Rigidbody2D>().SetPosition(vec2(10.f, 0));

		//Entity entity = CreateSandSprite("enemy_fighter.png", "enemy_fighter_mask.png");
		////entity.Add<FireWeaponAfterDelay>(player, Weapon::LASER, 1.f);
		//entity.Add<TurnTwoardsTarget>(target);
		////entity.Add<Flocker>(3.f, 1.f, 7.f);
		////entity.Add<ExplodeNearTarget>(player);
		////entity.Add<Mesh>(GenerateCircle(16, 5.f));

		for (int i = 0; i < 0; i++)
		{
			Entity entity;

			switch (get_rand(1))
			{
				case 0: // fighter
				{
					entity = CreateSandSprite("enemy_fighter.png", "enemy_fighter_mask.png");
					entity.Add<FireWeaponAfterDelay>(player, Weapon::LASER, 1.f + get_randc(.3f));
					entity.Add<TurnTwoardsTarget>(target);
					entity.Add<Flocker>();

					break;
				}

				case 1: // bomb
				{
					entity = CreateSandSprite("enemy_bomb.png", "enemy_bomb_mask.png");
					entity.Add<TurnTwoardsTarget>(target);

					break;
				}

				case 2: // station
				{
					entity = CreateSandSprite("enemy_station.png", "enemy_station_mask.png");
					entity.Add<TurnTwoardsTarget>(target);
					entity.Add<Flocker>();

					break;
				}
			}

			Transform2D& transform = entity.Get<Transform2D>();
			transform.position = get_randc(20.f, 20.f);

			entity.Add<Rigidbody2D>(transform).SetFixedRotation(true);
		}
	}

	// should make sand cut the collider sprite as well, this will make the colliders much simopler
	// I think, if this doesnt happen then the sprite will become incorrect on the first hit

	Entity CreateSandSprite(const std::string& path, const std::string& collider_mask_path)
	{
		auto [sand, physics] = m_app.GetModules<SandWorld, PhysicsWorld>();

		r<Texture> sprite = mkr<Texture>(_p(path), false);
		r<Texture> mask   = mkr<Texture>(_p(collider_mask_path), false);

		assert(sprite->Length() == mask->Length());

		Transform2D transform;
		transform.scale.x = sprite->Width()  / sand.worldScale.x;
		transform.scale.y = sprite->Height() / sand.worldScale.y;

		// collider should be 8 bit mask texture
		// if its not then the color texture can be used actually so maybe this is fine

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

		r<Texture> sprite = mkr<Texture>(_p(path), false);
		r<Texture> mask   = mkr<Texture>(_p(path_mask), false);
		
		assert(sprite->Length() == mask->Length());

		Transform2D transform;
		transform.scale = vec2(sprite->Width(), sprite->Height()) * sand.worldScale;

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
