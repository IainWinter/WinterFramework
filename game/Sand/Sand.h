#pragma once

#include "Rendering.h"
#include "Entity.h"
#include "Physics.h"
#include "Leveling.h"
#include "Windowing.h"
#include "Events.h"
#include "ext/Time.h"
#include "ext/flood_fill.h"
#include "ext/marching_cubes.h"
#include "ext/Components.h"
#include <vector>
#include <functional>
#include <unordered_set>

#include "Components/KeepOnScreen.h"

#include "Sand/SandEvents.h"
#include "Sand/SandComponents.h"

// everywhere LevelManager is needed should be inside of a System
// because Systems are owned by whatever level, so it's more contained
// will result in less bugs

struct SandWorld
{
	Entity display;

	vec2 worldScaleInit; // scale of meters to cells, needed for sprite cutting
	vec2 worldScale; // scale of meters to cells
	vec2 screenOffset;
	ivec2 screenSize;

	r<Target> screenRead;
	//r<Target> screenWrite; // to make copying to cpu not stall?
	Entity lineProjectiles;

	SandWorld() = default;

	SandWorld(int w, int h, int camScaleX, int camScaleY)
	{
		screenRead = std::make_shared<Target>(false);
		screenRead->Add(Target::aColor, w, h, Texture::uINT_32, false);

		//screenWrite = std::make_shared<Target>(false);
		//screenWrite->Add(Target::aColor, w, h, Texture::uINT_32, false);

		//spriteTarget->Add(Target::aDepth, w, h, 1);
		
		Mesh LPmesh = Mesh(false);
		LPmesh.Add<vec2>(Mesh::aPosition, {});
		LPmesh.Add<vec4>(Mesh::aColor, {});
		LPmesh.topology = Mesh::tLines;

		lineProjectiles = LevelManager::CurrentLevel()->CreateEntity().AddAll(Transform2D(), LPmesh);

		ResizeWorld(w, h, camScaleX, camScaleY);
		worldScaleInit = worldScale;
	}

	void ResizeWorld(int width, int height, int camScaleX, int camScaleY)
	{
		screenRead->Resize(width, height);
		//screenWrite->Resize(width, height);
		screenSize = vec2(width, height);
		screenOffset = screenSize / 2;

		worldScale.x = width  / camScaleX / 2; // by 2 bc mesh does from -1 to 1
		worldScale.y = height / camScaleY / 2;
	}

	Entity CreateCell(float x, float y, Color color, float vx = 0, float vy = 0, float dampen = 20.f)
	{
		return LevelManager::CurrentLevel()->CreateEntity().AddAll(Cell { ToScreenPos(vec2(x, y)), vec2(vx, vy), dampen, color });
	}

	Entity CreateCell(vec2 position, vec2 velocity, Color color)
	{
		return LevelManager::CurrentLevel()->CreateEntity().AddAll(Cell { ToScreenPos(position), velocity, 1.f, color });
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
		return *(ivec4*)screenRead->Get(Target::aColor)->At<int>(pos.x, pos.y);
	}

	vec2 ToScreenPos(const vec2& pos) const
	{
		return pos * worldScale;
	}

	bool OnScreen(ivec2 pos) const
	{
		pos += screenOffset; 
		return pos.x >= 0 && pos.y >= 0 && pos.x < screenSize.x && pos.y < screenSize.y;
	}

	// yes moves
	//  no copys
	SandWorld(SandWorld&& move) = default;
	SandWorld& operator=(SandWorld&& move) = default;
	SandWorld(const SandWorld& copy) = delete;
	SandWorld& operator=(const SandWorld& copy) = delete;
};

// rendering for collision info

struct SandCollisionInfoRenderer
{
	ShaderProgram m_shader;
	Mesh          m_quad;

	// this gets run multiple times... should save static stuff like shaders
	// drop raii just use init function or something

	SandCollisionInfoRenderer()
	{
		m_quad.Add<vec2>(Mesh::aPosition, { vec2(-1, -1), vec2(1, -1), vec2(1, 1), vec2(-1, 1) });
		m_quad.Add<vec2>(Mesh::aTextureCoord, { vec2(0, 0), vec2(1, 0), vec2(1, 1), vec2(0, 1) });
		m_quad.Add<int>(Mesh::aIndexBuffer, { 0, 1, 2, 0, 2, 3});

		const char* source_vert =
			"#version 330 core\n"
			"layout (location = 0) in vec2 pos;"
			"layout (location = 1) in vec2 uv;"

			"out vec2 TexCoords;"

			"uniform mat4 model;"
			"uniform mat4 projection;"

			"void main()"
			"{"
				"TexCoords = uv;"
				"gl_Position = projection * model * vec4(pos, 0.0, 1.0);"
			"}";

		const char* source_frag =
			"#version 330 core\n"
			"in vec2 TexCoords;"

			"out ivec4 spriteId;" // sprite (x, y) (entity index), (alpha for if its even there)

			"uniform sampler2D colliderMask;"
			"uniform vec2 spriteSize;"
			"uniform int spriteIndex;"

			"void main()"
			"{"
				"vec4 mask = texture(colliderMask, TexCoords);"
				"if (mask.a > .7) mask.a = 1.f;" // round up for health thing

				"if (mask.a == 0) discard;"

				"spriteId = ivec4(TexCoords * spriteSize, spriteIndex, 1);"
			"}";

		m_shader.Add(ShaderProgram::sVertex, source_vert);
		m_shader.Add(ShaderProgram::sFragment, source_frag);
	}

	void Begin(Camera& camera, r<Target> target)
	{
		if (target) target->Use();
		else Target::UseDefault();

		m_shader.Use();
		m_shader.Set("projection", camera.Projection());

		gl(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL));
		gl(glClearColor(0, 0, 0, 0));
		gl(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
	}

	void DrawCollisionInfo(const Transform2D& transform, SandSprite& sprite, int spriteIndex)
	{
		Texture& mask = sprite.Get();

		m_shader.Set("model", transform.World());
		m_shader.Set("colliderMask", mask);
		m_shader.Set("spriteSize", vec2(mask.Width(), mask.Height()));
		m_shader.Set("spriteIndex", spriteIndex);

		m_quad.Draw();
	}
};

// sand update

// render all the tiles to a texture, could chunk but regolith is a single screen game so dont worry about this for now
// render bullets

struct Sand_System_Update : System<Sand_System_Update>
{
	struct Islands
	{
		std::vector<std::vector<int>> coreIslands;
		std::vector<std::vector<int>> otherIslands;

		size_t Count() const
		{
			return coreIslands.size() + otherIslands.size();
		}
	};

	struct ToSplit
	{
		Entity hit;
		Entity projectile;
		ivec2 locationInSprite;
		bool operator==(const ToSplit& t) const { return (hit.Id() == t.hit.Id()); }
	};
	struct ToSplitHash { size_t operator()(const ToSplit& t) const { return t.hit.Id(); } };

	// queue events to not exe a split on every collision, could be multiple per frame on same obj
	std::unordered_set<ToSplit, ToSplitHash> toSplit;

	// queue creating colliders to multithread in Update
	std::unordered_set<Entity> toCreateCollider;
	
	// for the sprite index in the shader, indexes into this array
	std::vector<Entity> tileCache; 

	SandCollisionInfoRenderer maskRender;

	void Init()
	{
		Attach<event_SpawnSandCell>();
		Attach<event_WindowResize>();
		Attach<event_SandCellCollision>();
		Attach<event_SandAddSprite>();
		Attach<event_SandTurnSpriteToDust>();
	}

	void on(event_SpawnSandCell& e)
	{
		Entity entity = GetModule<SandWorld>().CreateCell(e.position, e.velocity, e.color);
		if (e.life > 0.f) entity.Add<CellLife>(e.life);
		if (e.onCreate) e.onCreate(entity);
	}

	void on(event_WindowResize& e)
	{
		auto [sand, camera] = GetModules<SandWorld, Camera>();
		sand.ResizeWorld(e.width, e.height, camera.w, camera.h);
	}

	void on(event_SandCellCollision& e)
	{
		// assumes sprite and mask are same size

		auto [sprite, mask] = e.entity.GetAll<Sprite, SandSprite>();

		int index = sprite.Get().Index32(e.hitPosInSprite.x, e.hitPosInSprite.y);
		Color color = sprite.Get().At(index);
		
		sprite.Get().At(index).a = 0;
		mask  .Get().At(index).a = 0;
		mask.cellCount -= 1;

		toSplit.insert({
			e.entity,
			e.projectile,
			e.hitPosInSprite
		});

		Cell cell = e.projectile.Get<Cell>();

		Send(event_SpawnSandCell{cell.pos, cell.vel + get_randn(length(cell.vel) / 2.f), color, .5f });
	}

	void on(event_SandAddSprite& e)
	{
		e.entity.Add<KeepOnScreen>();

		Texture& sprite = e.entity.Get<Sprite>().Get();
		Transform2D& transform = e.entity.Get<Transform2D>();
		transform.scale.x = sprite.Width()  / GetModule<SandWorld>().worldScaleInit.x;
		transform.scale.y = sprite.Height() / GetModule<SandWorld>().worldScaleInit.y;

		SandSprite& sandSprite = e.entity.Get<SandSprite>();

		auto [coreIndex, filled, isHealth, hasAny] = GetCorePixels(e.entity.Get<Sprite>().m_source);

		sandSprite.core = coreIndex;
		sandSprite.hasCore = isHealth;
		sandSprite.density = e.density;
		sandSprite.cellCount = filled.size();

		// setup collider if requested

		if (!e.entity.Has<Rigidbody2D>())
		{
			Rigidbody2D& body = GetModule<PhysicsWorld>().AddEntity(e.entity);
			body.SetVelocity(e.velocity);
			body.SetAngularVelocity(e.aVelocity);
			
			SendNow(event_Sand_CreateCollider{ e.entity });
		}
	}

	void on(event_SandTurnSpriteToDust& e)
	{
		r<Texture> sprite = e.entity.Get<Sprite>().m_source;
		vec2 projectileVel = get_randn(100.f);
		vec2 mid = vec2(sprite->Width(), sprite->Height()) / 2.f;

		ExplodeSpriteIntoDust(std::get<1>(GetCorePixels(sprite)), sprite, e.entity.Get<Transform2D>(), mid, projectileVel);

		e.entity.Destroy();
	}

	void Update()
	{
		auto [camera, sand, window] = GetModules<Camera, SandWorld, Window>();

		// Sprite update

		printf("Number of tiles to split %d\n", toSplit.size());

		//TaskSyncPoint syncPoint(toSplit.size());

		for (auto& split : toSplit)
		{
			if (!split.hit.IsAlive())
			{
				printf("invalid entity in toSplit list!\n");
				continue; // should investigate why this sometimes is invalid
			}

			if (split.hit.Get<SandSprite>().cellCount <= 0)
			{
				split.hit.Destroy();
				continue;
			}

			//Thread([&]()
			//{
				SplitSprite(split.hit, split.projectile);
				//syncPoint.Tick();
			//});
		}

		//syncPoint.BlockUntilZero();

		toSplit.clear();
		tileCache.clear();

		// Sand Update

		//vec2 reverseCamScale = sand.worldScale / vec2(window.m_config.Width, window.m_config.Height);/s		//sand.display.Get<Transform2D>().sx = reverseCamScale.x;
		//display.Get<Tran<Transform2D>().sy = reverseCamScale.y;	sand.tileCache.clear(); // bad for a system to touch state like this... kinda a hack only because entity handle cant turn into a u32

		// Render tiles to hidden target

		maskRender.Begin(camera, sand.screenRead);

		int spriteIndex = 0;
		for (auto [e, transform, sandSprite] : QueryWithEntity<Transform2D, SandSprite>())
		{
			maskRender.DrawCollisionInfo(transform, sandSprite, spriteIndex);
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

		Texture& collisionMaskRender = *sand.screenRead->Get(Target::aColor); // sprite info / collision info, probally an index (or entity index) to an array of structs
		collisionMaskRender.SendToHost();

		std::vector<vec2> verticesOfLines;
		std::vector<vec4>   colorsOfLines;

		for (auto [e, cell] : QueryWithEntity<Cell>())
		{
			auto [a, b] = DrawLine(sand, collisionMaskRender, e, cell);
			float dist = distance(a, b);

			vec2 smallest = vec2(1.f / sand.worldScale.x, 1.f / sand.worldScale.y) * 1.f;
			if (dist < length(smallest))
			{
				b += smallest;
			}

			verticesOfLines.push_back(a);
			verticesOfLines.push_back(b);

			colorsOfLines.push_back(cell.color.as_v4());
			colorsOfLines.push_back(cell.color.as_v4());
		}

		Mesh& lines = sand.lineProjectiles.Get<Mesh>();
		lines.Get(Mesh::aPosition)->Set(verticesOfLines);
		lines.Get(Mesh::aColor)   ->Set(colorsOfLines);

		//std::swap(sand.screenRead, sand.screenWrite);
	}

private:

	// return begin and end of line
	std::pair<vec2, vec2> DrawLine(SandWorld& sand, Texture& collisionInfo, Entity e, Cell& cell)
	{
		vec2 delta = cell.vel / cell.dampen * Time::DeltaTime();
		vec2 origin = cell.pos;
		vec2 current = cell.pos;

		float distance = glm::length(delta);
		delta /= distance;

		bool isProjectile = e.Has<CellProjectile>();

		for (int i = 0; i < ceil(distance); i++)
		{
			ivec2 raster = floor(current);

			if (!sand.OnScreen(raster))
			{
				continue; // break if going offscreen
			}

			if (isProjectile && sand.CollidePixel(raster))
			{
				const ivec4& spriteInfo = sand.GetCollisionInfo(raster);
				ivec2 positionInSprite = ivec2(spriteInfo[0], spriteInfo[1]);
				int tileIndex = spriteInfo[2];

				Entity tileEntity = tileCache.at(tileIndex);

				if (!tileEntity.IsAlive()) continue;

				if (tileEntity.Id() != e.Get<CellProjectile>().owner)
				{
					SandSprite& sprite = tileEntity.Get<SandSprite>();
					if (!sprite.invulnerable)
					{
						SendNow(event_SandCellCollision { tileEntity, e, positionInSprite }); // should defer
						
						int& health = e.Get<CellProjectile>().health;
						health -= sprite.cellStrength;
						if (health <= 0)
						{
							e.Get<CellLife>().life = 0;
							break;
						}

						// turn projectile a little
						vec2& v = e.Get<Cell>().vel;
						float d = length(v);
						v = normalize(v + get_randn(1000.f)) * d;

						delta = normalize(cell.vel / cell.dampen * Time::DeltaTime());
					}

					else // stop and delete projectile
					{
						e.Get<CellLife>().life = 0;
						break;
					}
				}
			}

			//sand.DrawPixel(display, raster, cell.color);
			current += delta;
			cell.pos = current;
		}

		return { origin / sand.worldScale, current / sand.worldScale };
	}

	// start splitting functions into clear peices

	void SplitSprite(Entity splitMe, Entity projectile)
	{
		auto [transform, body, drawSprite, sandSprite] = splitMe.GetAll<Transform2D, Rigidbody2D, Sprite, SandSprite>();

		r<Texture> sprite = drawSprite.m_source;
		r<Texture> mask   = sandSprite.colliderMask;

		vec2 projectileVel = projectile.Get<Cell>().vel;
		vec2 mid = vec2(mask->Width(), mask->Height()) / 2.f;

		// this should go somewhere else, like in event
		//body.ApplyForce(projectileVel / 1.f, (vec2(projectileLocation) - mid) / sand.worldScale);

		Islands islands = GetIslands(splitMe.Get<SandSprite>());

		if (islands.Count() > 1)
		{
			if (islands.coreIslands.size() > 1) // this is the core and the bits surrounding
			{
				Send(event_SpawnExplosion{transform.position, 50.f});
			}

			for (const std::vector<int>& island : islands.coreIslands)
				SplitFromIsland(island, sprite, mask, transform, body, mid, projectileVel);

			for (const std::vector<int>& island : islands.otherIslands)
			{
				SplitFromIsland(island, sprite, mask, transform, body, mid, projectileVel);

				for (const int& index : island)
				{
					sprite->At(index) = Color(0);
					mask  ->At(index) = Color(0);
				}
			}

			//Coroutine([=]() 
			//{
				GetModule<PhysicsWorld>().Remove(splitMe.Get<Rigidbody2D>()); // todo: add listener to physics obj
				sprite->Cleanup();
				mask->Cleanup();
				splitMe.Destroy();

				//return true;
			//});
		}
	}

	//
	//
	// All these are helpers functions for sprite splitting
	//
	//

	// core pixels

	// returns only health pixels, or all pixels if there are none
	// a health pixel has an alpha not equal to 0 or 255, could make a third texture for this, but seems unnessesary, no sprites have opacity as of now...
	std::tuple<std::vector<int>, std::vector<int>, bool, bool> GetCorePixels(const r<Texture>& texture)
	{
		std::vector<int> filled, core;

		u32* itr = (u32*)texture->Pixels();
		u32* end = itr + texture->Length();

		for (int i = 0; itr != end; itr++, i++)
		{
			u8 alpha = Color(*itr).a;

			if (alpha > 0) filled.push_back(i);
			if (alpha > 0 && alpha < 255) core.push_back(i);
		}

		if (core.size() == 0)
		{
			return { {}, filled, false, filled.size() > 0 };
		}

		return { core, filled, true, true };
	}

	// splitting

	void SplitFromIsland(const std::vector<int>& island, r<Texture> sprite, r<Texture> mask, Transform2D transform, Rigidbody2D& body, vec2 mid, vec2 projectileVel)
	{
		auto [minX, minY, maxX, maxY] = GetBoundingBoxOfIsland(island, mask->Width());

		if (island.size() < 15)
		{
			ExplodeSpriteIntoDust(island, sprite, transform, mid, projectileVel);
		}

		else
		{
			auto split = SplitSpriteInTwo(island, sprite, mask, transform, mid, minX, minY, maxX, maxY);
			
			//Coroutine([=]()
			//{
				// nice names lol
				
				auto [t, s, ss] = split;
				Entity e = CreateEntity().AddAll(t, Sprite(s), SandSprite(ss));
				
				vec2  v = body.GetVelocity();
				float a = body.GetAngularVelocity();
				float d = 10.f;//body.GetCollider()->GetDensity();

				SendNow(event_SandAddSprite { e, v, a, d });
						
				//return true;
			//});
		}
	}

	std::tuple<int, int, int, int> GetBoundingBoxOfIsland(const std::vector<int>& island, int width)
	{
		int minX =  INT_MAX;
		int minY =  INT_MAX;
		int maxX = -INT_MAX;
		int maxY = -INT_MAX;

		for (const int& index : island)
		{
			auto [x, y] = get_xy(index, width);
			if (x < minX) minX = x;
			if (y < minY) minY = y;
			if (x > maxX) maxX = x;
			if (y > maxY) maxY = y;
		}

		return { minX, minY, maxX, maxY };
	}

	void ExplodeSpriteIntoDust(const std::vector<int>& island, r<Texture>& sprite, const Transform2D& transform, vec2 mid, vec2 projectileVel)
	{
		SandWorld& sand = GetModule<SandWorld>(); // non threadsafe read

		for (const int& index : island)
		{
			auto [x, y] = get_xy(index, sprite->Width());

			vec2 pos = (vec2(x, y) - mid) / 10.f;
			rotate(pos, transform.rotation);
			pos += transform.position;

			Color& color = sprite->At(x, y);
			vec2 offset = 1.f / sand.worldScaleInit; // non threadsafe read

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

			Send(event_SpawnSandCell{ pos, vels[0], color, get_rand(.3f) + .1f });
			Send(event_SpawnSandCell{ pos, vels[1], color, get_rand(.3f) + .1f });
			Send(event_SpawnSandCell{ pos, vels[2], color, get_rand(.3f) + .1f });
			Send(event_SpawnSandCell{ pos, vels[3], color, get_rand(.3f) + .1f });
		}
	}

	std::tuple<Transform2D, r<Texture>, r<Texture>> SplitSpriteInTwo(const std::vector<int>& island, r<Texture>& sprite, r<Texture>& mask, Transform2D transform, vec2 mid, int minX, int minY, int maxX, int maxY)
	{
		// copy old data to new texture

		r<Texture> splitTexture = mkr<Texture>(maxX - minX + 1, maxY - minY + 1, Texture::uRGBA, false);
		r<Texture> splitMask    = mkr<Texture>(maxX - minX + 1, maxY - minY + 1, Texture::uRGBA, false); // could be uR
						
		splitTexture->ClearHost();
		splitMask   ->ClearHost();

		for (const int& index : island)
		{
			auto [x, y] = get_xy(index, sprite->Width());  // this math doesnt need to transform to xy
														   // simplify index = (x - minX) + (y - minY) * width
			// set color & mask

			splitTexture->At(x - minX, y - minY) = sprite->At(x, y);
			splitMask   ->At(x - minX, y - minY) = mask  ->At(x, y);
		}

		// create new tile
		// this only works for the new tile, not if the orignaldis is remade...

		// place the new sprites in their relitive locations

		vec2 midNew = vec2(minX + maxX + 1, minY + maxY + 1) / 2.f;
		vec2 offset = 2.f * rotate(midNew - mid, transform.rotation);

		transform.position += offset / GetModule<SandWorld>().worldScaleInit;

		return { transform, splitTexture, splitMask };
	}

	// flood fill

	Islands GetIslands(const SandSprite& sprite)
	{
		Islands islands;

		std::vector<flood_fill_cell_state> state = GetSpriteStates(sprite.colliderMask);

		if (sprite.hasCore)
		for (int seed : sprite.core)
		{
			AddSingleIsland(seed, sprite.colliderMask, state, islands.coreIslands);
		}

		// then we need to search for any leftover islands and split those off, maybe there needs to be a differnece between core groups and non core groups
		// if there are 2 core groups, means core is cut in half and should explode
		// all other islands should follow normal rules

		for (int i = 0; i < state.size(); i++)
		{
			if (state.at(i) == flood_fill_cell_state::FILLED)
			{
				AddSingleIsland(i, sprite.colliderMask, state, islands.otherIslands);
			}
		}

		return islands;
	}

	std::vector<flood_fill_cell_state> GetSpriteStates(const r<Texture>& mask)
	{
		return flood_fill_get_states_from_array<u32>(
			(u32*)mask->Pixels(), mask->Length(), [](const u32& x) { return Color(x).a > 0; }
		);
	}

	void AddSingleIsland(int seed, const r<Texture>& mask, std::vector<flood_fill_cell_state>& state, std::vector<std::vector<int>>& islands)
	{
		auto island = flood_fill(seed, mask->Width(), mask->Height(), state);
		if (island.size() > 0)
		{
			islands.emplace_back(std::move(island));
		}
	}

	// colliders

	// if a collider was set
	bool SetPolygonColliderOnSprite(Entity e, const r<Texture>& mask, float density)
	{
		assert(mask->Channels() == 4 && "collider mask needs to be 32 bit right now");
		//assert(e.Has<Transform2D, Rigidbody2D>());

		Transform2D& tran = e.Get<Transform2D>();
		Rigidbody2D& body = e.Get<Rigidbody2D>();
		
		auto polygons = MakePolygonFromField<u32>(
			(u32*)mask->Pixels(),
			mask->Width(),
			mask->Height(),
			[](const u32& color) { return (color & 0xff000000) > 0; }
		);

		body.RemoveColliders();

		for (const std::vector<vec2>& polygon : polygons.first)
		{
			b2PolygonShape shape;
			for (int i = 0; i < polygon.size(); i++)
			{
				shape.m_vertices[i] = _tb(polygon.at(i) * tran.scale);
			}
			shape.Set(shape.m_vertices, polygon.size());

			assert(polygon.size() < 12 && "hitbox library genereated a polygon with more than 12 verts, could expand b2 limit or put a limit on the methods in hitbox lib");
			body.AddCollider(shape, density);
		}

		// debug
		// wont work if entity has Mesh
		// need to make a model class with a list of Meshs + Transforms

		//if (e.Has<Mesh>()) e.Remove<Mesh>();

		//std::vector<vec2> debug_mesh;
		//for (const std::vector<vec2>& polygon : polygons.first) for (const vec2& v : polygon) debug_mesh.push_back(v);
		//e.Add<Mesh>();
		//Mesh& mesh = e.Get<Mesh>();
		//mesh.topology = Mesh::tLoops;
		//mesh.Add(Mesh::aPosition, debug_mesh);

		return polygons.first.size() > 0;
	}
};

// print debug shape in console
//for (int i = 0; i < sprite.colliderMask->Width(); i++)
//{
//	for (int j = 0; j < sprite.colliderMask->Height(); j++)
//	{
//		printf(state.at(i + j * sprite.colliderMask->Width()) == flood_fill_cell_state::FILLED ? "." : " ");
//	}
//	printf("\n");
//}
//printf("\n");