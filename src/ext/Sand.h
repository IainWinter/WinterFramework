#pragma once

#include "Rendering.h"
#include "Entity.h"
#include "Physics.h"
#include "ext/Time.h"
#include "ext/flood_fill.h"
#include <vector>
#include <functional>
#include <unordered_set>

struct Cell
{
	vec2 pos;
	vec2 vel;
	float dampen = 20;
	Color color;
};

struct CellLife
{
	float life;
};

struct SandSprite
{
	bool hasChanged = false;
	Texture colliderMask;
	SandSprite() = default;
	SandSprite(const Texture& collider) : colliderMask(collider) {}
};

struct event_SandCellCollision
{
	Entity projectile;
	Entity sprite;
	ivec2 hitPosInSprite;
};

struct event_SandAddSprite
{
	Entity entity;
	vec2 velocity = vec2(0.f, 0.f);
	float aVelocity = 0.f;
};

struct SandWorld
{
	Entity display;

	vec2 worldScale; // scale of meters to cells
	ivec2 screenSize;
	vec2 screenOffset;

	r<Target> screen;
	std::vector<Entity> tileCache; // for the sprite index in the shader, indexes into this array

	SandWorld() = default;

	SandWorld(int w, int h, int camScaleX, int camScaleY)
	{
		screen = std::make_shared<Target>();

		//spriteTarget->Add(Target::aDepth, w, h, 1);
		screen->Add(Target::aColor, w, h, Texture::uRGBA, false);
		screen->Add(Target::aColor1, w, h, Texture::uINT_32, false);

		display = GetWorld().Create();
		display.Add<Transform2D>(0, 0, 0, camScaleX, camScaleY, 0);
		display.Add<Sprite>(screen->Get(Target::aColor));
		display.Add<Renderable>();

		worldScale.x = w / camScaleX / 2; // by 2 bc mesh does from -1 to 1
		worldScale.y = h / camScaleY / 2;

		screenSize.x = w;
		screenSize.y = h;

		screenOffset = screenSize / 2;
	}

	~SandWorld()
	{
		display.Destroy();
	}

	Entity CreateCell(float x, float y, Color color, float vx = 0, float vy = 0, float dampen = 20.f)
	{
		return GetWorld().Create().AddAll(Cell{ vec2(x * worldScale.x, y * worldScale.y), vec2(vx, vy), dampen, color });
	}

	void DrawLine(Texture& display, Texture& collisionInfo, Entity e, Cell& cell)
	{
		vec2 delta = cell.vel / cell.dampen * Time::DeltaTime();
		vec2 current = cell.pos;

		float distance = glm::length(delta);
		delta /= distance;

		for (int i = 0; i < ceil(distance); i++)
		{
			ivec2 raster = floor(current + screenOffset);

			if (!OnScreen(raster))
			{
				// break if going offscreen
				continue;
			}

			if (CollidePixel(collisionInfo, raster))
			{

				int* spriteInfo = collisionInfo.At<int>(raster.x, raster.y);
				ivec2 positionInSprite = ivec2(spriteInfo[0], spriteInfo[1]);
				int tileIndex = spriteInfo[2];

				// should defer
				events().send(event_SandCellCollision { e, tileCache.at(tileIndex), positionInSprite });
				//entities_defer().destroy(e);
			}

			DrawPixel(display, floor(current + screenOffset), cell.color);
			current += delta;
			cell.pos = current;
		}
	}

	void DrawPixel(Texture& display, ivec2 pos, Color c)
	{
		display.At(pos.x, pos.y) = c;
	}

	bool CollidePixel(Texture& collisionInfo, ivec2 pos)
	{
		return collisionInfo.At<int>(pos.x, pos.y)[3] > 0;
	}

	bool OnScreen(ivec2 pos)
	{
		return pos.x > 0 && pos.y > 0 && pos.x < screenSize.x && pos.y < screenSize.y;
	}
};

// sand update

// render all the tiles to a texture, could chunk but regolith is a single screen game so dont worry about this for now
// render bullets

struct Sand_System_Update : System
{
	std::unordered_set<Entity > toSplit;

	 Sand_System_Update()
	 { 
		 events().attach<event_SandCellCollision>(this);
		 events().attach<event_SandAddSprite>(this);
	 }
	~Sand_System_Update() { events().detach(this); }

	void on(event_SandCellCollision& e)
	{
		vec2& vel = e.projectile.Get<Cell>().vel;
		float speed = length(vel);
		vel.x += get_rand(400) - 200;
		vel.y += get_rand(400) - 200;
		vel = normalize(vel) * speed;

		e.sprite.Get<Sprite>().Get().At(e.hitPosInSprite.x, e.hitPosInSprite.y).a = 0;
		e.sprite.Get<SandSprite>().hasChanged = true;

		if (toSplit.find(e.sprite) == toSplit.end())
		{
			toSplit.insert(e.sprite);
		}
	}

	void on(event_SandAddSprite& e)
	{
		Texture& sprite = e.entity.Get<Sprite>().Get();
		Transform2D& transform = e.entity.Get<Transform2D>();
		transform.sx = sprite.Width()  / Get<SandWorld>().worldScale.x;
		transform.sy = sprite.Height() / Get<SandWorld>().worldScale.y;

		// setup collider

		SandSprite& sandSprite = e.entity.Get<SandSprite>();

		assert(sandSprite.colliderMask.Channels() == 4 && "collider mask needs to be 32 bit right now, in future should be 8");

		auto polygons = MakePolygonFromField<u32>(
			(u32*)sandSprite.colliderMask.Pixels(),
			sandSprite.colliderMask.Width(),
			sandSprite.colliderMask.Height(),
			[](const u32& color) { return (color & 0xff000000) > 0; }
		);

		Rigidbody2D& body = Get<PhysicsWorld>().AddEntity(e.entity);
		body.SetVelocity(e.velocity);
		body.SetAngularVelocity(e.aVelocity);

		vec2 scale = vec2(transform.sx, transform.sy);

		for (const std::vector<vec2>& polygon : polygons.first)
		{
			b2PolygonShape shape;
			for (int i = 0; i < polygon.size(); i++)
			{
				shape.m_vertices[i] = _tb(polygon.at(i) * scale);
			}
			shape.Set(shape.m_vertices, polygon.size());

			body.AddCollider(shape);
		}

		// debug
		std::vector<vec2> debug_mesh; 
		for (const std::vector<vec2>& polygon : polygons.first) for (const vec2& v : polygon) debug_mesh.push_back(v);
		e.entity.Add<Mesh>();
		Mesh& mesh = e.entity.Get<Mesh>();
		mesh.Add(Mesh::aPosition, debug_mesh);
	}

	void Update()
	{
		auto [render, camera, sand, window] = Get<SpriteRenderer2D, Camera, SandWorld, Window>();

		// Sprite update

		// split sprites that needed it
		// this about multi-threading

		for (Entity splitMe : toSplit)
		{
			Texture& sprite = splitMe.Get<Sprite>().Get();
			int length = sprite.Width() * sprite.Height();

			printf("splitting sprite with length %d\n", length);

			std::vector<flood_fill_cell_state> state = flood_fill_get_states_from_array<u32>(
				(u32*)sprite.Pixels(), length, [](const u32& x) { return (x & 0xff000000) > 0; }
			);

			//for (int i = 0; i < sprite.Width(); i++)
			//{
			//	for (int j = 0; j < sprite.Height(); j++)
			//	{
			//		printf(state.at(i + j * sprite.Width()) == flood_fill_cell_state::FILLED ? "." : " ");
			//	}
			//	printf("\n");
			//}
			//printf("\n");

			std::vector<std::vector<int>> islands;

			for (int seed = 0; seed < length; seed++) // slow could use 'active pixels' cached list
			{
				std::vector<int> island = flood_fill(seed, sprite.Width(), sprite.Height(), state);
				if (island.size() > 0)
				{
					islands.emplace_back(std::move(island));
				}
			}

			if (islands.size() > 1)
			{
				for (const std::vector<int>& island : islands)
				{
					if (island.size() < 25)
					{
						// explode sprite
						continue;
					}

					// copy old data to new texture

					int minX =  INT_MAX;
					int minY =  INT_MAX;
					int maxX = -INT_MAX;
					int maxY = -INT_MAX;

					for (const int& index : island)
					{
						auto [x, y] = get_xy(index, sprite.Width());
						if (x < minX) minX = x;
						if (y < minY) minY = y;
						if (x > maxX) maxX = x;
						if (y > maxY) maxY = y;
					}

					Texture splitTexture = Texture(maxX - minX + 1, maxY - minY + 1, Texture::uRGBA, false);
					splitTexture.Clear();

					for (const int& index : island)
					{
						auto [x, y] = get_xy(index, sprite.Width()); // this math doesnt need to transform to xy
																	 // simplify index = (x - minX) + (y - minY) * width
						
						Color& to = splitTexture.At(x - minX, y - minY);
						Color& from = sprite.At(x, y);

						to = from;
						from = Color(0, 0, 0, 0);
					}

					// create new tile
					// this only works for the new tile, not if the orignaldis is remade...

					// place the new sprites in their relitive locations

					Transform2D splitTransform = splitMe.Get<Transform2D>();

					vec2 midOld = vec2(sprite.Width(), sprite.Height()) / 2.f;    // width/height because it's 0-width/height
					vec2 midNew = vec2(minX + maxX + 1, minY + maxY + 1) / 2.f;   // avergae of min and max bc min might not be 0, +1 because maxY is index not size
					vec2 offset = 2.f * rotate(midNew - midOld, splitTransform.r);
					
					splitTransform.x += offset.x / sand.worldScale.x;
					splitTransform.y += offset.y / sand.worldScale.y;

					Entity splitOff = GetWorld().Create().AddAll(splitTransform, Sprite(splitTexture), SandSprite(splitTexture));

					vec3 vel;
					if (splitMe.Has<Rigidbody2D>())
					{
						vel.x = splitMe.Get<Rigidbody2D>().GetVelocity().x;
						vel.y = splitMe.Get<Rigidbody2D>().GetVelocity().y;
						vel.z = splitMe.Get<Rigidbody2D>().GetAngularVelocity();
					}

					//vel.x += get_rand(.1f) - .05f;
					//vel.y += get_rand(.1f) - .05f;
					//vel.z += get_rand(.5f) - .025f;

					events().send(event_SandAddSprite {splitOff, vec2(vel.x, vel.y), vel.z});

					// this has some memory issue
				}

				sprite.Cleanup();
				Get<PhysicsWorld>().Remove(splitMe.Get<Rigidbody2D>()); // todo: add listener to physics obj
				splitMe.Destroy();
			}
		}

		toSplit.clear();
		sand.tileCache.clear();

		// Sand Update

		//vec2 reverseCamScale = sand.worldScale / vec2(window.m_config.Width, window.m_config.Height);/s		//sand.display.Get<Transform2D>().sx = reverseCamScale.x;
		//display.Get<Tran<Transform2D>().sy = reverseCamScale.y;	sand.tileCache.clear(); // bad for a system to touch state like this... kinda a hack only because entity handle cant turn into a u32

		// Render tiles to hidden target

		render.Begin(camera, sand.screen);
		render.Clear(Color(0));

		int spriteIndex = 0;
		for (auto [e, transform, sprite, sandSprite] : QueryWithEntity<Transform2D, Sprite, SandSprite>())
		{
			if (sandSprite.hasChanged)
			{
				sandSprite.hasChanged = false;
				sprite.Get().SendToDevice();
			}

			render.m_shader.Set("spriteIndex", spriteIndex);
			sand.tileCache.push_back(e);
			spriteIndex += 1;

			render.DrawSprite(transform, sprite.Get());
		}

		// Cell update

		Texture& color = *sand.screen->Get(Target::aColor);
		Texture& sInfo = *sand.screen->Get(Target::aColor1); // sprite info / collision info, probally an index (or entity index) to an array of structs

		color.SendToHost();
		sInfo.SendToHost();

		for (auto [e, cell] : QueryWithEntity<Cell>())
		{
			sand.DrawLine(color, sInfo, e, cell);
		}

		color.SendToDevice();
	}
};

struct Sand_LifeUpdateSystem : System
{
	void Update()
	{
		for (auto [e, time] : QueryWithEntity<CellLife>())
		{
			time.life -= Time::DeltaTime();
			if (time.life <= 0.f)
			{
				e.Destroy();
			}
		}
	}
};