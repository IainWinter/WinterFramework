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

struct EventLoggingSystem : System<EventLoggingSystem>
{
	void Init()
	{
		Attach<event_Shutdown>();
		Attach<event_WindowResize>();
		Attach<event_Mouse>();
		Attach<event_Input>();
	}

	void on(event_Shutdown& e)
	{
		printf(
			"[Event] Recieved: event_Shutdown {}\n");
	}

	void on(event_WindowResize& e)
	{
		printf(
			"[Event] Recieved: event_WindowResize {"
			"\n\twidth %d"
			"\n\theight %d"
			"\n}\n", e.width, e.height);
	}

	void on(event_Mouse& e)
	{	
		printf(
			"[Event] Recieved: event_Mouse {"
			"\n\tpixel_x %d"
			"\n\tpixel_y %d"
			"\n\tscreen_x %f"
			"\n\tscreen_y %f"
			"\n\tvel_x %f"
			"\n\tvel_y %f"
			"\n\tbutton_left %d"
			"\n\tbutton_middle %d"
			"\n\tbutton_right %d"
			"\n\tbutton_x1 %d"
			"\n\tbutton_x2 %d"
			"\n\tbutton_repeat %d"
			"\n}\n", e.pixel_x, e.pixel_y, e.screen_x, e.screen_y, 
				     e.vel_x, e.vel_y, e.button_left, e.button_middle,
				     e.button_right, e.button_x1, e.button_x2, e.button_repeat);
	}

	void on(event_Input& e)
	{
		static std::unordered_map<InputName, const char*> names = 
		{
			{ InputName::_NONE, "None" },
			{ InputName::UP,    "Up" },
			{ InputName::DOWN,  "Down" },
			{ InputName::RIGHT, "Right" },
			{ InputName::LEFT,  "Left" },
		};

		printf(
			"[Event] Recieved: event_Input {"
			"\n\tname %s"
			"\n\tstate %f"
			"\n}\n", names.at(e.name), e.state);
	}
};

struct PhysicsInterpolationSystem : SystemBase
{
	float m_acc = 0.f;

	void Update() override
	{
		m_acc += Time::DeltaTime();
		float ratio = clamp(m_acc / Time::RawFixedTime(), 0.f, 1.f);

		for (auto [transform, body] : Query<Transform2D, Rigidbody2D>())
		{
			if (!body.InWorld()) continue;

			transform.x = lerp(body.LastTransform.x, body.GetPosition().x, ratio);
			transform.y = lerp(body.LastTransform.y, body.GetPosition().y, ratio);
			transform.r = lerp(body.LastTransform.r, body.GetAngle(),      ratio);
		}
	}

	void FixedUpdate() override
	{
		m_acc = 0;

		for (auto [transform, body] : Query<Transform2D, Rigidbody2D>())
		{
			if (!body.InWorld()) continue;

			body.LastTransform.x = body.GetPosition().x;
			body.LastTransform.y = body.GetPosition().y;
			body.LastTransform.r = body.GetAngle();
		}
	}
};

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

struct SpriteRenderer2DSystem : SystemBase
{
	void Update() override
	{
		auto [camera, render] = GetModules<Camera, SpriteRenderer2D>();

		render.Begin(camera);
		render.Clear();
		for (auto [transform, sprite, _] : Query<Transform2D, Sprite, Renderable>())
		{
			render.DrawSprite(transform, sprite);
		}
	}
};

struct TriangleRenderer2DSystem : SystemBase
{
	void Update() override
	{
		auto [camera, render] = GetModules<Camera, TriangleRenderer2D>();

		render.Begin(camera, false);
		for (auto [transform, mesh] : Query<Transform2D, Mesh>())
		{
			render.DrawMesh(transform, mesh);
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

		ImGui::Begin("#");
		ImGui::SliderFloat2("pos", (float*)&body.GetPosition(), -10, 10);
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

		level->AddSystem(PhysicsInterpolationSystem());
		level->AddSystem(CharacterController());
		level->AddSystem(Sand_System_Update());
		level->AddSystem(SpriteRenderer2DSystem());
		//level->AddSystem(EventLoggingSystem());
		//level->AddSystem(ForceTwoardwsMouseSystem());

		m_app.GetModule<LevelManager>().InitLevel(level);
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