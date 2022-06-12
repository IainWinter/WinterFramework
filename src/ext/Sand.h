#pragma once

#include "Rendering.h"
#include "Entity.h"
#include "Physics.h"
#include "Leveling.h"
#include "Windowing.h"
#include "ext/Time.h"
#include "ext/flood_fill.h"
#include "ext/marching_cubes.h"
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
	u32 owner; // for no damage to who fired
};

struct CellLife
{
	float life;
};

struct SandSprite
{
	float density = 100.f;
	bool invulnerable = false;
	r<Texture> colliderMask;
	std::vector<int> core; // if this list has items, these are the only cells to floodfill

	Texture& Get() { return *colliderMask; }

	SandSprite() = default;
	SandSprite(const Texture& collider) : colliderMask(std::make_shared<Texture>(collider)) {}
};

struct event_SandCellCollision
{
	Entity entity;
	Entity projectile;
	ivec2 hitPosInSprite;
};

// cretea a mesh collider is entity has no Rigidbody component
struct event_SandAddSprite
{
	Entity entity;
	vec2 velocity = vec2(0.f, 0.f);
	float aVelocity = 0.f;
	float density = 0;
};

struct event_SpawnSandCell
{
	vec2 position;
	vec2 velocity;
	Color color;

	std::function<void(Entity)> onCreate;
};

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

	r<Target> screen;
	Entity lineProjectiles;

	SandWorld() = default;

	SandWorld(int w, int h, int camScaleX, int camScaleY)
	{
		screen = std::make_shared<Target>(false);
		screen->Add(Target::aColor, w, h, Texture::uINT_32, false);
		//spriteTarget->Add(Target::aDepth, w, h, 1);
		
		Mesh LPmesh = Mesh(false);
		LPmesh.Add<vec2>(Mesh::aPosition, {});
		LPmesh.topology = Mesh::tLines;

		lineProjectiles = LevelManager::CurrentLevel()->CreateEntity().AddAll(Transform2D(), LPmesh);

		ResizeWorld(w, h, camScaleX, camScaleY);
		worldScaleInit = worldScale;
	}

	void ResizeWorld(int width, int height, int camScaleX, int camScaleY)
	{
		screen->Resize(width, height);
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
		return *(ivec4*)screen->Get(Target::aColor)->At<int>(pos.x, pos.y);
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
				"spriteId = ivec4(TexCoords * spriteSize, spriteIndex, mask.a > 0);"
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
	
	// for the sprite index in the shader, indexes into this array
	std::vector<Entity> tileCache; 

	SandCollisionInfoRenderer maskRender;

	void Init()
	{
		Attach<event_SpawnSandCell>();
		Attach<event_WindowResize>();
		Attach<event_SandCellCollision>();
		Attach<event_SandAddSprite>();
	}

	void on(event_SpawnSandCell& e)
	{
		Entity entity = GetModule<SandWorld>().CreateCell(e.position, e.velocity, e.color);
		e.onCreate(entity);
	}

	void on(event_WindowResize& e)
	{
		auto [sand, camera] = GetModules<SandWorld, Camera>();
		sand.ResizeWorld(e.width, e.height, camera.w, camera.h);
	}

	void on(event_SandCellCollision& e)
	{
		vec2& vel = e.projectile.Get<Cell>().vel;
		float speed = length(vel);
		vel.x += get_rand(400) - 200;
		vel.y += get_rand(400) - 200;
		vel = normalize(vel) * speed;

		// assumes sprite and mask are same size

		auto [sprite, mask] = e.entity.GetAll<Sprite, SandSprite>();

		int index = sprite.Get().Index(e.hitPosInSprite.x, e.hitPosInSprite.y);
		sprite.Get().At(index).a = 0;
		mask  .Get().At(index).a = 0;

		toSplit.insert({
			e.entity,
			e.projectile,
			e.hitPosInSprite
		});
	}

	void on(event_SandAddSprite& e)
	{
		Texture& sprite = e.entity.Get<Sprite>().Get();
		Transform2D& transform = e.entity.Get<Transform2D>();
		transform.scale.x = sprite.Width()  / GetModule<SandWorld>().worldScaleInit.x;
		transform.scale.y = sprite.Height() / GetModule<SandWorld>().worldScaleInit.y;

		SandSprite& sandSprite = e.entity.Get<SandSprite>();

		sandSprite.core = GetCorePixels(e.entity.Get<Sprite>().m_source);

		// setup collider if requested

		if (!e.entity.Has<Rigidbody2D>())
		{
			Rigidbody2D& body = GetModule<PhysicsWorld>().AddEntity(e.entity);
			body.SetVelocity(e.velocity);
			body.SetAngularVelocity(e.aVelocity);
		
			assert(sandSprite.colliderMask->Channels() == 4 && "collider mask needs to be 32 bit right now, in future should be 8");

			auto polygons = MakePolygonFromField<u32>(
				(u32*)sandSprite.colliderMask->Pixels(),
				sandSprite.colliderMask->Width(),
				sandSprite.colliderMask->Height(),
				[](const u32& color) { return (color & 0xff000000) > 0; }
			);

			for (const std::vector<vec2>& polygon : polygons.first)
			{
				b2PolygonShape shape;
				for (int i = 0; i < polygon.size(); i++)
				{
					shape.m_vertices[i] = _tb(polygon.at(i) * transform.scale);
				}
				shape.Set(shape.m_vertices, polygon.size());

				assert(polygon.size() < 12 && "hitbox library genereated a polygon with more than 12 verts, could expand b2 limit or put a limit on the methods in hitbox lib");
				body.AddCollider(shape, e.density);
			}

			// debug
			// wont work if entity has Mesh
			// need to make a model class with a list of Meshs + Transforms
			//std::vector<vec2> debug_mesh; 
			//for (const std::vector<vec2>& polygon : polygons.first) for (const vec2& v : polygon) debug_mesh.push_back(v);
			//e.entity.Add<Mesh>();
			//Mesh& mesh = e.entity.Get<Mesh>();
			//mesh.Add(Mesh::aPosition, debug_mesh);
		}
	}

	void Update()
	{
		auto [camera, sand, window] = GetModules<Camera, SandWorld, Window>();

		// Sprite update

		for (auto [splitMe, projectile, projectileLocation] : toSplit)
		{
			auto [transform, body, drawSprite, sandSprite] = splitMe.GetAll<Transform2D, Rigidbody2D, Sprite, SandSprite>();

			r<Texture> sprite = drawSprite.m_source;
			r<Texture> mask   = sandSprite.colliderMask;

			vec2 projectileVel = projectile.Get<Cell>().vel;
			vec2 midOld = vec2(mask->Width(), mask->Height()) / 2.f;

			// this should go somewhere else, like in event
			body.ApplyForce(projectileVel / 1.f, (vec2(projectileLocation) - midOld) / sand.worldScale);

			Islands islands = GetIslands(splitMe.Get<SandSprite>());

			if (islands.Count() > 1)
			{
				for (const std::vector<int>& island : islands.coreIslands)
				{
					SplitFromIsland(island, sprite, mask, transform, body, midOld, projectileVel);
				}

				for (const std::vector<int>& island : islands.otherIslands)
				{
					SplitFromIsland(island, sprite, mask, transform, body, midOld, projectileVel);
				}

				GetModule<PhysicsWorld>().Remove(body); // todo: add listener to physics obj
				sprite->Cleanup();
				mask->Cleanup();
				splitMe.Destroy();
			}

			else
			{
				sandSprite.core = GetCorePixels(sprite);
			}
		}

		toSplit.clear();
		tileCache.clear();

		// Sand Update

		//vec2 reverseCamScale = sand.worldScale / vec2(window.m_config.Width, window.m_config.Height);/s		//sand.display.Get<Transform2D>().sx = reverseCamScale.x;
		//display.Get<Tran<Transform2D>().sy = reverseCamScale.y;	sand.tileCache.clear(); // bad for a system to touch state like this... kinda a hack only because entity handle cant turn into a u32

		// Render tiles to hidden target

		maskRender.Begin(camera, sand.screen);

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

		Texture& collisionMaskRender = *sand.screen->Get(Target::aColor); // sprite info / collision info, probally an index (or entity index) to an array of structs
		collisionMaskRender.SendToHost();

		std::vector<vec2> verticesOfLines;

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
		}

		Mesh& lines = sand.lineProjectiles.Get<Mesh>();
		lines.Get(Mesh::aPosition)->Set(verticesOfLines);
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

				if (!tileEntity.Get<SandSprite>().invulnerable)
				{
					if (tileEntity.Id() != e.Get<CellProjectile>().owner)
					{
						SendNow(event_SandCellCollision { tileEntity, e, positionInSprite }); // should defer
					}

					vec2& v = e.Get<Cell>().vel;
					v = normalize(v + get_rand(10.f, 10.f)) * length(v);
				}
			}

			//sand.DrawPixel(display, raster, cell.color);
			current += delta;
			cell.pos = current;
		}

		return { origin / sand.worldScale, current / sand.worldScale };
	}

	// start splitting functions into clear peices

	// core pixels

	// returns only health pixels, or all pixels if there are none
	// a health pixel has an alpha not equal to 0 or 255, could make a third texture for this, but seems unnessesary, no sprites have opacity as of now...
	std::vector<int> GetCorePixels(const r<Texture>& texture)
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

		if (core.size() == 0) core = filled; // use all filled if no core, this is for rocks and broken pieces

		assert(core.size() > 0 && "empty core");

		return core;
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
			Entity splitOff = SplitSprite(island, sprite, mask, transform, mid, minX, minY, maxX, maxY);

			vec2  v = body.GetVelocity();
			float a = body.GetAngularVelocity();
			float d = 10.f;//body.GetCollider()->GetDensity();

			//vec2 pveln = normalize(projectileVel);

			//vel.x += get_randc(.1f) + pveln.x;
			//vel.y += get_randc(.1f) + pveln.y;
			//vel.z += get_randc(.5f);

			SendNow(event_SandAddSprite { splitOff, v, a, d });
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

	void ExplodeSpriteIntoDust(const std::vector<int>& island, const r<Texture>& sprite, const Transform2D& transform, vec2 mid, vec2 projectileVel)
	{
		SandWorld& sand = GetModule<SandWorld>();

		for (const int& index : island)
		{
			auto [x, y] = get_xy(index, sprite->Width());

			vec2 pos = (vec2(x, y) - mid) / 10.f;
			rotate(pos, transform.rotation);
			pos += transform.position;

			Color color = sprite->At(x, y);
			vec2 offset = 1.f / sand.worldScaleInit;

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
	}

	Entity SplitSprite(const std::vector<int>& island, r<Texture>& sprite, r<Texture>& mask, Transform2D transform, vec2 mid, int minX, int minY, int maxX, int maxY)
	{
		// copy old data to new texture

		Texture splitTexture = Texture(maxX - minX + 1, maxY - minY + 1, Texture::uRGBA, false);
		Texture splitMask    = Texture(maxX - minX + 1, maxY - minY + 1, Texture::uRGBA, false); // could be uR
						
		splitTexture.ClearHost();
		splitMask.ClearHost();

		for (const int& index : island)
		{
			auto [x, y] = get_xy(index, sprite->Width());  // this math doesnt need to transform to xy
															// simplify index = (x - minX) + (y - minY) * width
			// set color

			Color& to = splitTexture.At(x - minX, y - minY);
			Color& from = sprite->At(x, y);

			to = from;
			from = Color(0, 0, 0, 0);

			// set mask

			Color& toMask = splitMask.At(x - minX, y - minY);
			Color& fromMask = mask->At(x, y);

			toMask = fromMask;
			fromMask = Color(0, 0, 0, 0);
		}

		// create new tile
		// this only works for the new tile, not if the orignaldis is remade...

		// place the new sprites in their relitive locations

		vec2 midNew = vec2(minX + maxX + 1, minY + maxY + 1) / 2.f;
		vec2 offset = 2.f * rotate(midNew - mid, transform.rotation);

		transform.position += offset / GetModule<SandWorld>().worldScaleInit;

		return LevelManager::CurrentLevel()->CreateEntity().AddAll(
			transform, 
			Sprite(splitTexture), 
			SandSprite(splitMask)
		);
	}

	// flood fill

	Islands GetIslands(const SandSprite& sprite)
	{
		Islands islands;

		std::vector<flood_fill_cell_state> state = GetSpriteStates(sprite.colliderMask);

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