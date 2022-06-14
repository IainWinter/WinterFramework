#include "EngineLoop.h"
#include "Common.h"
#include "Rendering.h"

#include "Sand.h"
#include "ext/Time.h"
#include "ext/MeshGenerators.h"
#include "ext/Components.h"

#include "imgui/implot.h"

// systems

#include "Systems//PlayerController.h"
#include "Systems/FlockingMovement.h"
#include "Systems/TurnTwoardsTarget.h"
#include "Systems/ExplodeNearTarget.h"
#include "Systems/ExplosionSpawner.h"
#include "Systems/EnemyController.h"
#include "Systems/KeepOnScreen.h"

#include "ext/systems/PhysicsInterpolation.h"
#include "ext/systems/SimpleSpriteRender.h"
#include "ext/systems/SimpleMeshRender.h"

//struct SandSpriteMaskRenderer : SystemBase
//{
//	void Update() override
//	{
//		auto [camera, render] = GetModules<Camera, SpriteRenderer2D>();
//
//		render.Begin(camera);
//		for (auto [transform, sprite] : Query<Transform2D, SandSprite>())
//		{
//			render.DrawSprite(transform, sprite.Get());
//		}
//	}
//};

struct MetricsSystem : System<MetricsSystem>
{
	std::vector<float> m_deltaTime;
	std::vector<float> m_fixedTimePoint;

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

		//ImPlot::SetNextAxesToFit();
		//ImPlot::BeginPlot("Times");
		//ImPlot::PlotLine("Delta time", m_deltaTime.data(), m_deltaTime.size());
		//ImPlot::EndPlot();

		ImGui::End();


		if (m_deltaTime.size() > 200) m_deltaTime.erase(m_deltaTime.begin()); // lots of copies
	}

	void on(event_RecordMetric& e)
	{
		switch (e.metric)
		{
			case TICK:            m_deltaTime     .push_back(e.value); break;
			case TICK_FIXED_TIME: m_fixedTimePoint.push_back(e.value); break;
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
		level->AddSystem(Sand_System_Update());
		level->AddSystem(SimpleSpriteRenderer2D());
		level->AddSystem(SimpleMeshRenderer2D());

		level->AddSystem(System_PlayerController());
		level->AddSystem(System_FlockingMovement());
		level->AddSystem(System_TurnTwoardsTarget());
		level->AddSystem(System_ExplodeNearTarget());
		level->AddSystem(System_ExplosionSpawner());
		level->AddSystem(System_EnemyController());
		level->AddSystem(System_KeepOnScreen());

		level->AddSystem(MetricsSystem());
		//level->AddSystem(SandSpriteMaskRenderer());
	}

	void ConfigureModules()
	{
		m_app.AddModule<SpriteRenderer2D>();
		m_app.AddModule<MeshRenderer2D>();
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

		Entity target = level->CreateEntity().AddAll(Transform2D(vec2(10.f, 0.f)));

		for (int i = 0; i < 2; i++)
		{
			Entity entity = CreateSandSprite("asteroid_mid_1.png", "asteroid_mid_1.png");
			//entity.Add<TurnTwoardsTarget>(target);
			//entity.Add<Flocker>();
			//entity.Add<ExplodeNearTarget>(player);
			//entity.Add<Mesh>(GenerateCircle(16, 5.f));
			
			entity.Get<Transform2D>().position = vec2(get_randc(20.f, 20.f));
			//entity.Get<Rigidbody2D>().SetPosition(vec2(get_randc(20.f, 20.f)));
			//entity.Get<Rigidbody2D>().SetVelocity(vec2(get_randc(20.f, 20.f)));
		}
	}

	// should make sand cut the collider sprite as well, this will make the colliders much simopler
	// I think, if this doesnt happen then the sprite will become incorrect on the first hit

	Entity CreateSandSprite(const std::string& path, const std::string& collider_mask_path)
	{
		auto [sand, physics] = m_app.GetModules<SandWorld, PhysicsWorld>();

		Texture sprite = Texture(_p(path), false);
		Texture mask   = Texture(_p(collider_mask_path), false);

		assert(sprite.Length() == mask.Length());


		Transform2D transform;
		transform.scale.x = sprite.Width()  / sand.worldScale.x;
		transform.scale.y = sprite.Height() / sand.worldScale.y;

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

		Texture sprite = Texture(_p(path), false);
		Texture mask   = Texture(_p(path_mask), false);
		
		assert(sprite.Length() == mask.Length());

		Transform2D transform;
		transform.scale = vec2(sprite.Width(), sprite.Height()) * sand.worldScale;

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

// rip

//struct Phase
//{
//	float StartDifficulty;
//	float EndDifficulty;
//	int EndScore;
//
//	Phase(float startDifficulty, float endDifficulty, int endScore)
//	{
//		StartDifficulty = startDifficulty;
//		EndDifficulty = endDifficulty;
//		EndScore = endScore;
//	}
//
//	float GetDifficulty(int relScore) 
//	{
//		// invalid deal with later ---> next step
//		// difficulty = (score < 0.5 ? 8 * score * score * score * score : 1 - pow(-2 * score + 2, 4) / 2);
//		float RelScore = relScore;
//		return lerp(StartDifficulty, EndDifficulty, RelScore);
//	}
//};
//
//struct Level
//{
//	// startpoint and endpoint are difficulty measurements
//	std::vector<Phase> phases;
//
//public: 
//	void AddPhase(float startDifficulty, float endDifficulty, int scoreLength)
//	{
//		assert(scoreLength > 0);
//
//		int currentScore = 0;
//		if (phases.size() > 0)
//		{
//			currentScore = phases.back().EndScore;
//		}
//
//		phases.push_back(Phase(startDifficulty, endDifficulty, currentScore + scoreLength));
//	}
//
//	Phase& GetPhase(int currentScore)
//	{
//		assert(phases.size() > 0);
//
//		for (int i = 0; i < phases.size(); i++)
//		{
//			if (currentScore < phases[i].EndScore)
//			{
//				return phases[i];
//			}
//		}
//
//		return phases.back();
//	}
//
//	void SpawnNext(int currentScore)
//	{
//		Phase& currentPhase = GetPhase(currentScore);
//	}
//
//	void SpawnFighter(float x, float y) {} 
//	void SpawnBomb(float x, float y) {}
//	void SpawnStation(float x, float y) {}
//	void SpawnBase(float x, float y) {}
//};
//
//void setup()
//{
//	Level currentLevel;
//
//	// create all the levels / load them from a file
//
//	// arbitrary score thresholds
//	// start at phase 1
//	// if score = 200* go to phase 2
//	// if score = 1000*  go to phase 3
//	// if score = 5000* go to phase 4
//	// if score = 15000* go to phase 5(boss)
//
//	currentLevel.AddPhase(0, 1, 200);
//	currentLevel.AddPhase(0, 1, 1000-200);
//	currentLevel.AddPhase(0, 1, 5000-1000);
//	currentLevel.AddPhase(0, 1, 15000-5000);
//
//	// Test Code
//	currentLevel.SpawnNext(100);
//	currentLevel.SpawnNext(1580);
//	currentLevel.SpawnNext(1000000);
//}
//
//
//
//bool loop()
//{
//
//
//}
