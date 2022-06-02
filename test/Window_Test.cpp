#include "Entry.h"
#include "Entity.h"
#include "Rendering.h"
#include "Windowing.h"
#include "Physics.h"
#include "ext/Time.h"
#include "ext/marching_cubes.h"
#include "ext/Sand.h"

#include <iostream>

using Order = void*;

struct Application
{
	Entity m_modules;

	float m_fixedTimeStepAcc;
	bool m_running;

	Application()
	{
		events().attach<event_Shutdown>(this);
		m_fixedTimeStepAcc = 0.f;
		m_running = true;
		m_modules = GetWorld().Create();
	}

	~Application()
	{
		events().detach(this);
		m_modules.Destroy();
	}

	template<typename _t>
	_t& Get()
	{
		return m_modules.Get<_t>();
	}

	bool Step(float deltaTime)
	{
		PhysicsWorld& world = m_modules.Get<PhysicsWorld>();

		m_fixedTimeStepAcc += deltaTime;
		if (m_fixedTimeStepAcc > Time::FixedTime())
		{
			m_fixedTimeStepAcc = 0;
			for (System* system : GetWorld().GetSystems())
			{
				system->FixedUpdate();
			}

			// This needs to be under here for the last transform loop
			// to interpolate correctly

			world.Step(Time::FixedTime());
		}

		for (System* system : GetWorld().GetSystems())
		{
			system->Update();
		}

		Window& window = m_modules.Get<Window>();

		window.BeginImgui();
		for (System* system : GetWorld().GetSystems())
		{
			system->UI();
		}
		window.EndImgui();

		window.EndFrame();
		window.PumpEvents();

		events_defer().execute();
		
		return m_running;
	}

	// these cant be in the ecs because they need an order of execution
	// this is where a jobs system comes in I think, I guess you make a strand
	// and at each level it can be multithreaded?
	// I guess each loop through the entities actually could queue work in a thread pool
	// and then exeacute all 100000 work items

	void on(event_Shutdown& e)
	{
		m_running = false;
	}
};

Application app; // for this simple test use global

Entity CreateSandSprite(const std::string& path, const std::string& collider_mask_path, Transform2D transform = {})
{
	Texture sprite = Texture(_p(path), false);
	SandWorld& sand = app.Get<SandWorld>();
	transform.sx = sprite.Width()  / sand.worldScale.x;
	transform.sy = sprite.Height() / sand.worldScale.y;

	// collider should be 8 bit mask texture

	Entity entity = app.Get<PhysicsWorld>().CreatePhysicsEntity(transform);
	entity.Add<SandSprite>(Texture(_p(collider_mask_path)));
	entity.Add<Sprite>(sprite);

	//events().send(event_SandAddSprite{ entity });

	return entity;
}

Entity CreateTexturedCircle(const std::string& path, Transform2D transform = {})
{
	Texture sprite = Texture(_p(path), false);
	SandWorld& sand = app.Get<SandWorld>();
	transform.sx = sprite.Width()  / sand.worldScale.x;
	transform.sy = sprite.Height() / sand.worldScale.y;

	Entity entity = app.Get<PhysicsWorld>().CreatePhysicsEntity(transform);
	entity.Add<SandSprite>();
	entity.Add<Sprite>(sprite);

	Rigidbody2D& body = entity.Get<Rigidbody2D>();

	b2CircleShape shape;
	shape.m_radius = std::max(transform.sx, transform.sy);
	body.AddCollider(shape);

	//events().send(event_SandAddSprite{ entity });

	return entity;
}

struct Player
{
	float speed = 300;
};

// I wonder if you could just declspec a templated type to
// test if it contained these functions and store their func pointers if
// they do
// system shsould never inherit so this should work
// the last engine got bogged down with excessive empty virtual functions

struct EventLoggingSystem : System
{
	EventLoggingSystem()
	{
		events().attach<event_Shutdown>(this);
		events().attach<event_WindowResize>(this);
		events().attach<event_Mouse>(this);
		events().attach<event_Input>(this);
	}

	~EventLoggingSystem()
	{
		events().detach(this);
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

struct PhysicsInterpolationSystem : System
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

struct ForceTwoardwsMouseSystem : System
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

struct SpriteRenderer2DSystem : System
{
	void Update() override
	{
		auto [camera, render] = *Query<Camera, SpriteRenderer2D>().begin();

		render.Begin(camera);
		render.Clear();
		for (auto [transform, sprite, _] : Query<Transform2D, Sprite, Renderable>())
		{
			render.DrawSprite(transform, sprite);
		}
	}
};

struct TriangleRenderer2DSystem : System
{
	void Update() override
	{
		auto [camera, render] = *Query<Camera, TriangleRenderer2D>().begin();

		render.Begin(camera, false);
		for (auto [transform, mesh] : Query<Transform2D, Mesh>())
		{
			render.DrawMesh(transform, mesh);
		}
	}
};

struct CharacterController : System
{
	float x = 0;
	float y = 0;
	float vx = 0;
	float vy = 0;
	float speed = 200;
	bool mouseDown = false;
	float fireTime = .05f;

	float mouseX, mouseY; // temp

	CharacterController()
	{
		events().attach<event_Input>(this);
		events().attach<event_Mouse>(this);
	}

	~CharacterController()
	{
		events().detach(this);
	}

	void Update() override
	{
		auto [sand] = *Query<SandWorld>().begin();
		auto [player, transform] = *Query<Player, Transform2D>().begin();

		fireTime -= Time::DeltaTime();
		if (mouseDown && fireTime < 0.f)
		{
			fireTime = .05f;

			sand.CreateCell(transform.x, transform.y + 5, Color(255, 255, 19), 
				x * 1500 + get_rand(200) - 10, y * 1500 + get_rand(020) - 10, 1).Add<CellLife>(5.f);
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

void setup_modules()
{
	WindowConfig windowConfig = {
		"Winter Framework Testbed", 1280, 720
	};

	app.m_modules.Add<Window>(windowConfig, &events());
	app.m_modules.Add<SpriteRenderer2D>();
	app.m_modules.Add<TriangleRenderer2D>();
	app.m_modules.Add<PhysicsWorld>();
	app.m_modules.Add<SandWorld>(1280, 720, 32, 18);
	app.m_modules.Add<Camera>(0, 0, 32, 18);
}

void setup_systems()
{
	GetWorld().AddSystem(PhysicsInterpolationSystem());
	GetWorld().AddSystem(CharacterController());
	
	GetWorld().AddSystem(Sand_LifeUpdateSystem());
	GetWorld().AddSystem(Sand_System_Update());

	GetWorld().AddSystem(SpriteRenderer2DSystem());
}

void setup_inputmapping()
{
	InputMapping& input = app.Get<Window>().m_input;

	input.m_keyboard[SDL_SCANCODE_W] = InputName::UP;
	input.m_keyboard[SDL_SCANCODE_S] = InputName::DOWN;
	input.m_keyboard[SDL_SCANCODE_D] = InputName::RIGHT;
	input.m_keyboard[SDL_SCANCODE_A] = InputName::LEFT;
}

void setup()
{
	setup_modules();
	setup_systems();
	setup_inputmapping();

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
	//CreateTexturedCircle("enemy_bomb.png", t);
	//CreateTexturedCircle("enemy_bomb.png", t);
	//CreateTexturedCircle("enemy_bomb.png", t);
	//CreateTexturedCircle("enemy_bomb.png", t);
	//CreateTexturedCircle("enemy_bomb.png", t);
	//CreateTexturedCircle("enemy_bomb.png", t);
	//CreateTexturedCircle("enemy_bomb.png", t);
	CreateTexturedCircle("enemy_bomb.png", t).Add<Player>();
}

bool loop()
{
	Time::UpdateTime();
	return app.Step(Time::DeltaTime());
}
