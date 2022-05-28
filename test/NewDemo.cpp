#include "Entry.h"

#include "Entity.h"
#include "Rendering.h"
#include "Windowing.h"
#include "Event.h"

entity app;

struct Mover
{
	Mover()
	{
		events().attach<event_Mouse>(this);
	}

	~Mover()
	{
		events().detach(this);
	}

	void on(event_Mouse& e)
	{
		auto [t] = entities().query<Transform2D, Sprite>().only<Transform2D>().first();
		t.x = e.screen_x;
		t.y = e.screen_y;
	}
};

void setup()
{
	WindowConfig config;
	config.Width = 1280;
	config.Height = 720;
	config.Title = "New Demo Window";

	app = entities().create()
		.add<Window>(config, &events())
		.add<SpriteRenderer2D>()
		.add<Mover>();

	entities().create()
		.add<Transform2D>()
		.add<Sprite>(Texture("C:/Users/Iain/3D Objects/banner.PNG", false));
}

bool loop()
{
	Camera cam;
	cam.h = -5;
	cam.w = 5;

	app.get<SpriteRenderer2D>().Begin(cam);
	app.get<SpriteRenderer2D>().Clear();
	for (auto [transform, sprite] : entities().query<Transform2D, Sprite>())
	{
		app.get<SpriteRenderer2D>().DrawSprite(transform, sprite);

		Texture& texture = sprite.Get();
		texture.Pixels()[get_rand(texture.Width() * texture.Height() * 4)] = get_rand(255);
		texture.SendToDevice();
	}

	app.get<Window>().EndFrame();
	app.get<Window>().PumpEvents();

	return true;
}
