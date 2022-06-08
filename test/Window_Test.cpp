#include "EngineLoop.h"

#include "Rendering.h"
#include "ext/marching_cubes.h"
#include "ext/Sand.h"

#include <iostream>

struct Player
{
	float speed = 300;
};

//enum EnemyType
//{
//	FIGHTER,
//	BOMB,
//	STATION,
//	BASE
//};
//
//struct event_SpawnEnemy
//{
//	EnemyType enemyType;
//};
//
//struct Enemy
//{
//
//};
//
//struct EnemySpawnnerSystem : System<EnemySpawnnerSystem>
//{
//	void Init()
//	{
//		Attach<event_SpawnEnemy>();
//	}
//
//	void Update()
//	{
//		// getDifficulty
//	}
//
//	void on(event_SpawnEnemy& e)
//	{
//		std::string spritePath;
//		Entity enemy = LevelManager::CurrentLevel()->CreateEntity().AddAll(Transform2D{}, Enemy{});
//
//		switch (e.enemyType)
//		{
//			case FIGHTER:
//			{
//				enemy.Add<Sprite>("enemy_fighter.png");
//				break;
//			}
//			case BOMB:
//			{
//				spritePath = "enemy_bomb.png";
//				break;
//			}
//			case STATION:
//			{
//				spritePath = "enemy_station.png";
//				break;
//			}
//			case BASE:
//			{
//				spritePath = "enemy_base.png";
//				break;
//			}
//		}
//	}
//};
//
//struct EnemyAISystem : SystemBase
//{
//	void Update()
//	{
//		for (auto [transform, enemy] : Query<Transform2D, Enemy>())
//		{
//
//		}
//	}
//};

#include "ext/systems/EventLogging.h"
#include "ext/systems/PhysicsInterpolation.h"
#include "ext/systems/SimpleSpriteRender.h"
#include "ext/systems/SimpleTriangleRender.h"

struct ForceTwoardwsMouseSystem : SystemBase
{
	void FixedUpdate() override
	{
		const Uint8* keystate = SDL_GetKeyboardState(nullptr);

		for (auto [transform, body] : Query<Transform2D, Rigidbody2D>())
		{
			if (!body.InWorld()) continue;

			float fx = 0 - transform.x;
			float fy = 0 - transform.y;

			body.ApplyForce(vec2(fx, fy));
		}
	}
};

struct CharacterController : System<CharacterController>
{
	float x = 0;
	float y = 0;
	float vx = 0;
	float vy = 0;
	float speed = 200;
	bool mouseDown = false;
	float fireTime = .05f;

	float mouseX, mouseY; // temp

	void Init()
	{
		Attach<event_Input>();
		Attach<event_Mouse>();
	}

	void Update() override
	{
		auto [sand] = GetModules<SandWorld>();
		auto [player, transform] = *Query<Player, Transform2D>().begin();

		fireTime -= Time::DeltaTime();
		if (mouseDown && fireTime < 0.f)
		{
			fireTime = .05f;

			sand.CreateCell(
				transform.x,
				transform.y + 5,
				Color(255, 255, 19),
				x * 1500 + get_rand(200) - 10,
				y * 1500 + get_rand(200) - 10, 1
			)
				.AddAll(CellLife{ 5.f }, CellProjectile {0});
		}
		
		//sand.CreateCell(mouseX, mouseY, Color(255, 100, 100));
	}

	void FixedUpdate() override
	{
		auto [_, body] = *Query<Player, Rigidbody2D>().begin();
		body.ApplyForce(vec2(vx * speed, vy * speed));
	}

	void UI() override
	{
		auto [body] = *Query<Rigidbody2D>().begin();

		float ts = Time::TimeScale();
		vec2 pos = body.GetPosition();

		ImGui::Begin("#");
		ImGui::SliderFloat2("pos", (float*)&pos, -10, 10);
		ImGui::SliderFloat("speed", &speed, 0, 1000);
		ImGui::SliderFloat("time", &ts, 0, 2);
		ImGui::Text("Deltatime: %f", Time::RawDeltaTime());
		ImGui::End();

		if (ts != Time::TimeScale())
		{
			Time::SetTimeScale(ts);
		}
	}

	void on(event_Input& e)
	{
		if (e.name == InputName::UP)    vy += e.state;
		if (e.name == InputName::DOWN)  vy -= e.state;
		if (e.name == InputName::RIGHT) vx += e.state;
		if (e.name == InputName::LEFT)  vx -= e.state;
	}

	void on(event_Mouse& e)
	{
		mouseDown = e.button_left;
		x = e.screen_x;
		y = e.screen_y;

		float d = sqrt(x*x + y*y);
		x /= d;
		y /= d;

		x *= 2;
		y *= 2;

		mouseX = e.screen_x;
		mouseY = e.screen_y;
	}
};

struct WindowTest : EngineLoop
{
	void _Init()
	{
		ConfigureWindow();
		ConfigureModules();
		ConfigureLevel();
		ConfigureInputMapping();

		CreateSomeTestEntities();
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
		level->AddSystem(CharacterController());
		level->AddSystem(Sand_System_Update());
		level->AddSystem(SimpleSpriteRenderer2D());
		//level->AddSystem(EventLoggingSystem());
		//level->AddSystem(ForceTwoardwsMouseSystem());
	}

	void ConfigureModules()
	{
		m_app.AddModule<SpriteRenderer2D>();
		m_app.AddModule<TriangleRenderer2D>();
		m_app.AddModule<SandWorld>(1280, 720, 32, 18);
		m_app.AddModule<Camera>(0, 0, 32, 18); // remove this
	}

	void ConfigureInputMapping()
	{
		InputMapping& input = m_app.GetModule<Window>().m_input;

		input.m_keyboard[SDL_SCANCODE_W] = InputName::UP;
		input.m_keyboard[SDL_SCANCODE_S] = InputName::DOWN;
		input.m_keyboard[SDL_SCANCODE_D] = InputName::RIGHT;
		input.m_keyboard[SDL_SCANCODE_A] = InputName::LEFT;
	}

	// remove these
	// should be called through events

	void CreateSomeTestEntities()
	{
		for (int i = 0; i < 1; i++)
		{
			Transform2D t;
			t.x = 10 + i * 10;
			CreateSandSprite("enemy_station.png", "enemy_station_mask.png", t);
		}

		Transform2D t;

		t.y = 10;
		CreateSandSprite("test_line.png", "test_line.png", t);
		t.y = 0;
		//CreateSandSprite("enemy_base.png", "enemy_base_mask.png", t);

		t.x = -10;
		CreateTexturedCircle("enemy_bomb.png", t).Add<Player>();
	}

	Entity CreateSandSprite(const std::string& path, const std::string& collider_mask_path, Transform2D transform = {})
	{
		auto [sand, physics] = m_app.GetModules<SandWorld, PhysicsWorld>();

		Texture sprite = Texture(_p(path), false);
		transform.sx = sprite.Width() / sand.worldScale.x;
		transform.sy = sprite.Height() / sand.worldScale.y;

		// collider should be 8 bit mask texture

		Entity entity = LevelManager::CurrentLevel()->CreateEntity();
		entity.Add<Transform2D>(transform);
		entity.Add<SandSprite>(Texture(_p(collider_mask_path)));
		entity.Add<Sprite>(sprite);

		physics.AddEntity(entity);

		return entity;
	}

	Entity CreateTexturedCircle(const std::string& path, Transform2D transform = {})
	{
		auto [sand, physics] = m_app.GetModules<SandWorld, PhysicsWorld>();

		Texture sprite = Texture(_p(path), false);
		transform.sx = sprite.Width() / sand.worldScale.x;
		transform.sy = sprite.Height() / sand.worldScale.y;

		Entity entity = LevelManager::CurrentLevel()->CreateEntity();
		entity.Add<Transform2D>(transform);
		entity.Add<SandSprite>();
		entity.Add<Sprite>(sprite);

		Rigidbody2D& body = physics.AddEntity(entity);

		b2CircleShape shape;
		shape.m_radius = std::max(transform.sx, transform.sy);
		body.AddCollider(shape);

		return entity;
	}
};

void setup()
{
	RunEngineLoop<WindowTest>();
}