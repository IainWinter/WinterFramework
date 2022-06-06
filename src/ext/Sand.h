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

struct CellProjectile
{
	int owner; // for no damage to who fired
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
	Entity sprite;
	Entity projectile;
	ivec2 hitPosInSprite;
};

struct event_SandAddSprite
{
	Entity entity;
	vec2 velocity = vec2(0.f, 0.f);
	float aVelocity = 0.f;
};

// everywhere LevelManager is needed should be inside of a System
// because Systems are owned by whatever level, so it's more contained
// will result in less bugs

struct SandWorld
{
	Entity display;

	vec2 worldScale; // scale of meters to cells
	ivec2 screenSize;
	vec2 screenOffset;

	r<Target> screen;

	SandWorld() = default;

	SandWorld(int w, int h, int camScaleX, int camScaleY)
	{
		screen = std::make_shared<Target>();

		//spriteTarget->Add(Target::aDepth, w, h, 1);
		screen->Add(Target::aColor, w, h, Texture::uRGBA, false);
		screen->Add(Target::aColor1, w, h, Texture::uINT_32, false);
		
		// see Windowing.h
		//screen->Add(Target::aDepth, w, h, Texture::uDEPTH, true);

		display = LevelManager::CurrentLevel()->CreateEntity();
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
		if (!display.IsAlive()) return; // moved
		display.Destroy();
	}

	Entity CreateCell(float x, float y, Color color, float vx = 0, float vy = 0, float dampen = 20.f)
	{
		return LevelManager::CurrentLevel()->CreateEntity()
			.AddAll(Cell { ToScreenPos(vec2(x, y)), vec2(vx, vy), dampen, color });
	}


	void DrawPixel(Texture& display, ivec2 pos, Color c)
	{
		pos += screenOffset;
		display.At(pos.x, pos.y) = c;
	}

	bool CollidePixel(ivec2 pos) const
	{
		return GetCollisionInfo(pos)[3] > 0;
	}

	const ivec4& GetCollisionInfo(ivec2 pos) const
	{
		pos += screenOffset;
		return *(ivec4*)screen->Get(Target::aColor1)->At<int>(pos.x, pos.y);
	}

	vec2 ToScreenPos(const vec2& pos) const
	{
		return pos * worldScale;
	}

	bool OnScreen(ivec2 pos) const
	{
		pos += screenOffset; 
		return pos.x > 0 && pos.y > 0 && pos.x < screenSize.x && pos.y < screenSize.y;
	}

	// yes moves
	//  no copys
	SandWorld(SandWorld&& move) = default;
	SandWorld& operator=(SandWorld&& move) = default;
	SandWorld(const SandWorld& copy) = delete;
	SandWorld& operator=(const SandWorld& copy) = delete;
};

// sand update

// render all the tiles to a texture, could chunk but regolith is a single screen game so dont worry about this for now
// render bullets

struct Sand_System_Update : System<Sand_System_Update>
{
	std::vector<event_SandCellCollision> toSplit; // queue events to not exe a split on every collision, could be multiple per frame on same obj
	std::vector<Entity> tileCache; // for the sprite index in the shader, indexes into this array

	void Init()
	{ 
		Attach<event_SandCellCollision>();
		Attach<event_SandAddSprite>();
	}

	void on(event_SandCellCollision& e)
	{
		vec2& vel = e.projectile.Get<Cell>().vel;
		float speed = length(vel);
		vel.x += get_rand(400) - 200;
		vel.y += get_rand(400) - 200;
		vel = normalize(vel) * speed;

		e.sprite.Get<Sprite>().Get().At(e.hitPosInSprite.x, e.hitPosInSprite.y).a = 0;
		e.sprite.Get<SandSprite>().hasChanged = true;

		bool alreadyHit = false;
		for (event_SandCellCollision& E : toSplit)
		{
			if (E.sprite == e.sprite)
			{
				alreadyHit = true;
				break;
			}
		}

		if (!alreadyHit)
		{
			toSplit.push_back(e);
		}
	}

	void on(event_SandAddSprite& e)
	{
		Texture& sprite = e.entity.Get<Sprite>().Get();
		Transform2D& transform = e.entity.Get<Transform2D>();
		transform.sx = sprite.Width()  / GetModule<SandWorld>().worldScale.x;
		transform.sy = sprite.Height() / GetModule<SandWorld>().worldScale.y;

		// setup collider

		SandSprite& sandSprite = e.entity.Get<SandSprite>();

		assert(sandSprite.colliderMask.Channels() == 4 && "collider mask needs to be 32 bit right now, in future should be 8");

		auto polygons = MakePolygonFromField<u32>(
			(u32*)sandSprite.colliderMask.Pixels(),
			sandSprite.colliderMask.Width(),
			sandSprite.colliderMask.Height(),
			[](const u32& color) { return (color & 0xff000000) > 0; }
		);

		Rigidbody2D& body = GetModule<PhysicsWorld>().AddEntity(e.entity);
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

			assert(polygon.size() < 9);
			body.AddCollider(shape, 1000.f);
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
		auto [render, camera, sand, window] = GetModules<SpriteRenderer2D, Camera, SandWorld, Window>();

		// Sprite update

		// split sprites that needed it
		// this about multi-threading

		for (auto [splitMe, projectile, projectileLocation] : toSplit)
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

			Rigidbody2D& body = splitMe.Get<Rigidbody2D>();
			vec2 projectileVel = projectile.Get<Cell>().vel;
			vec2 midOld = vec2(sprite.Width(), sprite.Height()) / 2.f; // width/height because it's 0-width/height

			body.ApplyForce(projectileVel / 10.f, (midOld - vec2(projectileLocation)) / sand.worldScale);

			if (islands.size() > 1)
			{
				for (const std::vector<int>& island : islands)
				{
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

					Transform2D splitTransform = splitMe.Get<Transform2D>();

					if (island.size() < 15)
					{
						for (const int& index : island)
						{
							auto [x, y] = get_xy(index, sprite.Width());

							vec2 pos = (vec2(x, y) - midOld) / 10.f;
							rotate(pos, splitTransform.r);
							pos += vec2(splitTransform.x, splitTransform.y);

							Color color = sprite.At(x, y);
							vec2 offset = 1.f / sand.worldScale;

							auto get_vel = [projectileVel]()
							{
								//float r = 100.f;
								//vec2 v = vec2(get_randc(r), get_randc(r));
								vec2 v = projectileVel;

								return v*.1f; // x .1 makes this faster??
							};

							vec2 vels[4] = {
								get_vel() + get_randc(50, 50),
								get_vel() + get_randc(50, 50),
								get_vel() + get_randc(50, 50),
								get_vel() + get_randc(50, 50)
							};

							sand.CreateCell(pos.x,            pos.y,            color, vels[0].x, vels[0].y, 1).Add<CellLife>(get_randc(1.f));
							sand.CreateCell(pos.x + offset.x, pos.y,            color, vels[1].x, vels[1].y, 1).Add<CellLife>(get_randc(1.f));
							sand.CreateCell(pos.x,            pos.y + offset.y, color, vels[2].x, vels[2].y, 1).Add<CellLife>(get_randc(1.f));
							sand.CreateCell(pos.x + offset.x, pos.y + offset.y, color, vels[3].x, vels[3].y, 1).Add<CellLife>(get_randc(1.f));
						}

						// need to find location in sand world of every pixel
						// a clean way to do this would be to find the minx, miny pixel and vec2 offset per pixel in sandworld for rotation

						// explode sprite
						continue;
					}

					// copy old data to new texture

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

					vec2 midNew = vec2(minX + maxX + 1, minY + maxY + 1) / 2.f;
					vec2 offset = 2.f * rotate(midNew - midOld, splitTransform.r);
					
					splitTransform.x += offset.x / sand.worldScale.x;
					splitTransform.y += offset.y / sand.worldScale.y;

					Entity splitOff = LevelManager::CurrentLevel()->CreateEntity()
						.AddAll(splitTransform, Sprite(splitTexture), SandSprite(splitTexture));

					vec3 vel;
					if (splitMe.Has<Rigidbody2D>())
					{
						vel.x = splitMe.Get<Rigidbody2D>().GetVelocity().x;
						vel.y = splitMe.Get<Rigidbody2D>().GetVelocity().y;
						vel.z = splitMe.Get<Rigidbody2D>().GetAngularVelocity();
					}

					vel.x += get_randc(.1f) + projectileVel.x / 1000.f;
					vel.y += get_randc(.1f) + projectileVel.y / 1000.f;
					vel.z += get_randc(.5f);

					SendNow(event_SandAddSprite {splitOff, vec2(vel.x, vel.y), vel.z});

					// this has some memory issue
				}

				sprite.Cleanup();
				GetModule<PhysicsWorld>().Remove(splitMe.Get<Rigidbody2D>()); // todo: add listener to physics obj
				splitMe.Destroy();
			}
		}

		toSplit.clear();
		tileCache.clear();

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
			render.DrawSprite(transform, sprite.Get());
			spriteIndex += 1;

			tileCache.push_back(e);
		}

		// Cell update

		for (auto [e, time] : QueryWithEntity<CellLife>())
		{
			time.life -= Time::DeltaTime();
			if (time.life <= 0.f)
			{
				e.Destroy();
			}
		}

		Texture& color = *sand.screen->Get(Target::aColor);
		Texture& sInfo = *sand.screen->Get(Target::aColor1); // sprite info / collision info, probally an index (or entity index) to an array of structs

		color.SendToHost();
		sInfo.SendToHost();

		for (auto [e, cell] : QueryWithEntity<Cell>())
		{
			if (length(cell.vel / cell.dampen * Time::DeltaTime()) > 1)
			{
				DrawLine(sand, color, sInfo, e, cell);
			}

			else
			{
				if (sand.OnScreen(cell.pos))
				{
					sand.DrawPixel(color, cell.pos, cell.color);
				}

				cell.pos += cell.vel * Time::DeltaTime();
			}
		}

		color.SendToDevice();
	}

	void DrawLine(SandWorld& sand, Texture& display, Texture& collisionInfo, Entity e, Cell& cell)
	{
		vec2 delta = cell.vel / cell.dampen * Time::DeltaTime();
		vec2 current = cell.pos;

		float distance = glm::length(delta);
		delta /= distance;

		bool isProjectile = e.Has<CellProjectile>();

		for (int i = 0; i < ceil(distance); i++)
		{
			ivec2 raster = floor(current);

			if (!sand.OnScreen(raster))
			{
				// break if going offscreen
				continue;
			}

			if (isProjectile && sand.CollidePixel(raster))
			{
				const ivec4& spriteInfo = sand.GetCollisionInfo(raster);
				ivec2 positionInSprite = ivec2(spriteInfo[0], spriteInfo[1]);
				int tileIndex = spriteInfo[2];

				// should defer
				SendNow(event_SandCellCollision{ tileCache.at(tileIndex), e, positionInSprite });
			}

			sand.DrawPixel(display, raster, cell.color);
			current += delta;
			cell.pos = current;
		}
	}
};