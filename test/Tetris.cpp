#include "Entry.h"
#include "Windowing.h"
#include "Rendering.h"
#include "ext/rendering/BatchSpriteRenderer.h"
#include "ext/Time.h"

Window window;
Camera camera;
BatchSpriteRenderer render;

constexpr int width  = 10;
constexpr int height = 20;

int grid[width][height];
float frameRate = 1 / 3.f;
float frameTimer = 0.f;

void TickGrid()
{
	for (int x = 0; x < width; x++)
	for (int y = 0; y < height; y++)
	{
		if (y != height - 1 && grid[x][y + 1] == 0)
		{
			std::swap(grid[x][y], grid[x][y + 1]);
		}
	}
}

void setup()
{
	camera = Camera(-5, -8, 5, 8);
	window = Window({ "Tetris" , 500, 800, });

	for (int x = 0; x < width; x++)
	for (int y = 0; y < height; y++)
	{
		grid[x][y] = get_rand(2);
	}
}

bool loop()
{
	Time::UpdateTime();

	Target::Clear(Color(0, 0, 0));
	render.Begin(camera);
	
	for (int x = 0; x < width; x++)
	for (int y = 0; y < height; y++)
	{
		if (grid[x][y] == 0) continue;

		Transform2D transform;
		transform.position = vec2(x+.5, y+.5);
		transform.scale = vec2(.49f);

		render.SubmitSprite(transform, Color(255, 0, 0));
	}

	frameTimer += Time::DeltaTime();
	if (frameTimer > frameRate)
	{
		frameTimer = 0.f;
		TickGrid();
	}

	render.Draw();

	window.EndFrame();
	window.PumpEvents();

	return window.IsOpen();
}