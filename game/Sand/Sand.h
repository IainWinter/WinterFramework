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
#include "Sand/SandHelpers.h"

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
		screenRead = mkr<Target>(false);
		screenRead->Add(Target::aColor, w, h, Texture::uINT_32, false);

		//screenWrite = std::mkr<Target>(false);
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
	r<ShaderProgram> m_shader;
	r<Mesh>          m_quad;

	// this gets run multiple times... should save static stuff like shaders
	// drop raii just use init function or something

	SandCollisionInfoRenderer()
	{
		m_shader = mkr<ShaderProgram>();
		m_quad   = mkr<Mesh>();

		m_quad->Add<vec2>(Mesh::aPosition, { vec2(-1, -1), vec2(1, -1), vec2(1, 1), vec2(-1, 1) });
		m_quad->Add<vec2>(Mesh::aTextureCoord, { vec2(0, 0), vec2(1, 0), vec2(1, 1), vec2(0, 1) });
		m_quad->Add<int>(Mesh::aIndexBuffer, { 0, 1, 2, 0, 2, 3});

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

		m_shader->Add(ShaderProgram::sVertex, source_vert);
		m_shader->Add(ShaderProgram::sFragment, source_frag);
	}

	void Begin(Camera& camera, r<Target> target)
	{
		if (target) target->Use();
		else Target::UseDefault();

		m_shader->Use();
		m_shader->Set("projection", camera.Projection());

		gl(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL));
		gl(glClearColor(0, 0, 0, 0));
		gl(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
	}

	void DrawCollisionInfo(const Transform2D& transform, SandSprite& sprite, int spriteIndex)
	{
		Texture& mask = sprite.Get();

		m_shader->Set("model", transform.World());
		m_shader->Set("colliderMask", mask);
		m_shader->Set("spriteSize", vec2(mask.Width(), mask.Height()));
		m_shader->Set("spriteIndex", spriteIndex);

		m_quad->Draw();
	}
};

// sand update

// render all the tiles to a texture, could chunk but regolith is a single screen game so dont worry about this for now
// render bullets

struct Sand_System_Update : System<Sand_System_Update>
{
	struct Islands
	{
		using Group = std::vector<std::vector<int>>;

		Group coreIslands;
		Group otherIslands;

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
		bool operator==(const ToSplit& t) const { return (hit.raw_id() == t.hit.raw_id()); }
	};
	struct ToSplitHash { size_t operator()(const ToSplit& t) const { return t.hit.raw_id(); } };

	// queue events to not exe a split on every collision, could be multiple per frame on same obj
	std::unordered_set<ToSplit, ToSplitHash> toSplit;

	// queue creating colliders to multithread in Update
	std::unordered_set<Entity> toCreateCollider;
	
	// for the sprite index in the shader, indexes into this array
	std::vector<Entity> tileCache; 

	SandCollisionInfoRenderer maskRender;

	void Init()
	{
		Attach<event_WindowResize>();
		Attach<event_SandCellCollision>();
		Attach<event_SandAddSprite>();
		Attach<event_Sand_CreateCell>();

		//maskRender = mkr<SandCollisionInfoRenderer>();
	}

	void on(event_Sand_CreateCell& e)
	{
		Cell c = e.cell;
		if (e.adjustPosition)
		{
			c.pos = GetModule<SandWorld>().ToScreenPos(c.pos);
		}

		Entity entity = CreateEntity().AddAll(c);
		
		if (e.onCreate)
		{
			e.onCreate(entity);
		}
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
		cell.vel = cell.vel / (get_rand(5.f) + 1.f);
		cell.vel += get_randn(length(cell.vel) / 4.f);
		cell.life = .2f;
		cell.color = color;
		Send(event_Sand_CreateCell(cell));
	}

	void on(event_SandAddSprite& e)
	{
		if (!e.entity.Has<KeepOnScreen>())
			e.entity.Add<KeepOnScreen>();

		Texture& sprite = e.entity.Get<Sprite>().Get();
		Transform2D& transform = e.entity.Get<Transform2D>();
		transform.scale.x = sprite.Width()  / GetModule<SandWorld>().worldScaleInit.x;
		transform.scale.y = sprite.Height() / GetModule<SandWorld>().worldScaleInit.y;

		SandSprite& sandSprite = e.entity.Get<SandSprite>();

		auto [coreIndex, filled, isHealth, hasAny] = GetCorePixels(e.entity.Get<Sprite>().source);

		sandSprite.core = coreIndex;
		sandSprite.density = e.density;
		sandSprite.cellCount = filled.size();

		// setup collider if requested

		Rigidbody2D& body = GetModule<PhysicsWorld>().AddEntity(e.entity);
		body.SetVelocity(e.velocity);
		body.SetAngularVelocity(e.aVelocity);

		if (body.GetColliderCount() == 0)
		{
			SendNow(event_Sand_CreateCollider{ e.entity });
		}
	}

	void Update()
	{
		auto [camera, sand, window] = GetModules<Camera, SandWorld, Window>();

		// Sprite update
		
		std::unordered_set<ToSplit, ToSplitHash> validToSplit;

		for (auto& split : toSplit)
		{
			if (split.hit.IsAlive())
			{
				int cellCount = split.hit.Get<SandSprite>().cellCount;

				     if (cellCount == 0) split.hit.Destroy();
				else if (cellCount < 15) Send(event_Sand_ExplodeToDust{split.hit});
				else                     validToSplit.insert(split);
			}

			else
			{
				printf("invalid entity in toSplit list!\n");
			}
		}

		TaskSyncPoint syncPoint(validToSplit.size());

		for (auto& split : validToSplit)
		{
			Thread([&]()
			{
				SplitSprite(split.hit, split.projectile);
				syncPoint.Tick();
			});
		}

		syncPoint.BlockUntilZero();

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

		Texture& collisionMaskRender = *sand.screenRead->Get(Target::aColor); // sprite info / collision info, probally an index (or entity index) to an array of structs
		collisionMaskRender.SendToHost();

		std::vector<vec2> verticesOfLines;
		std::vector<vec4>   colorsOfLines;

		for (auto [e, cell] : QueryWithEntity<Cell>())
		{
			cell.life -= Time::DeltaTime();
			if (cell.life <= 0.f)
			{
				e.DestroyAtEndOfFrame();
				continue;
			}

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
		vec2 delta = cell.vel * Time::DeltaTime();
		vec2 origin = cell.pos;

		float distance = glm::length(delta);
		delta /= distance;

		bool isProjectile = e.Has<CellProjectile>();

		for (int i = 0; i < ceil(distance); i++)
		{
			ivec2 raster = floor(cell.pos);

			if (isProjectile && sand.OnScreen(raster))
			{
				// trail
				CreateEntity().Add<Cell>(cell.pos, vec2(0.f, 0.f), cell.color, .1f);

				if (sand.CollidePixel(raster))
				{
					const ivec4& spriteInfo = sand.GetCollisionInfo(raster);
					ivec2 positionInSprite = ivec2(spriteInfo[0], spriteInfo[1]);
					int tileIndex = spriteInfo[2];

					Entity tileEntity = tileCache.at(tileIndex);
					CellProjectile& proj = e.Get<CellProjectile>();

					if (!tileEntity.IsAlive() || tileEntity.Id() == proj.owner) continue;

					SandSprite& sprite = tileEntity.Get<SandSprite>();
					
					if (sprite.invulnerable)
					{
						cell.life = 0;
						break;
					}
					
					SendNow(event_SandCellCollision{ tileEntity, e, positionInSprite }); // should defer

					int& health = e.Get<CellProjectile>().health;
					health -= sprite.cellStrength;
					
					if (health <= 0)
					{
						cell.life = 0;
						break;
					}

					// turn projectile a little
					vec2& v = cell.vel;
					float d = length(v);
					v = normalize(v + get_randn(d * proj.turnOnHitRate)) * d;
					delta = normalize(cell.vel * Time::DeltaTime());
				}
			}

			cell.pos += delta;
		}

		return { origin / sand.worldScale, cell.pos / sand.worldScale };
	}

	// start splitting functions into clear peices

	void SplitSprite(Entity splitMe, Entity projectile)
	{
		auto [transform, body, drawSprite, sandSprite] = splitMe.GetAll<Transform2D, Rigidbody2D, Sprite, SandSprite>();

		r<Texture> sprite = drawSprite.source;
		r<Texture> mask   = sandSprite.colliderMask;

		vec2 projectileVel = projectile.Get<Cell>().vel;
		vec2 mid = vec2(mask->Width(), mask->Height()) / 2.f;

		// this should go somewhere else, like in event
		//body.ApplyForce(projectileVel / 1.f, (vec2(projectileLocation) - mid) / sand.worldScale);

		Islands islands = GetIslands(splitMe.Get<SandSprite>());

		// Split if there are more than two islands (means there is a gap between two sets of cells in the sprite)
		// If an island contains the core, then its the main peice and should keep the components of the orignal sprite
		// If there are two islands that contain the core, this means it's been split and should explode
		// Any waother island is part of the larger armor, and can be split off by spawning a new entity

		if (islands.Count() > 1)
		{
			// this might need to change the center of mass?
			// atleast recalc colliders when the core hasnt been repasted

			if (islands.coreIslands.size() > 1) // this is the core and the bits surrounding
			{
				Send(event_SpawnExplosion{transform.position, 10.f});

				// effectivly remove core
				for (auto& core : islands.coreIslands) islands.otherIslands.emplace_back(std::move(core));
				islands.coreIslands.clear();
			}

			for (const std::vector<int>& island : islands.otherIslands)
			{
				SplitFromIsland(island, splitMe, projectile);

				for (const int& index : island)
				{
					sprite->At(index) = Color(0);
					mask  ->At(index) = Color(0);
				}

				sandSprite.cellCount -= island.size();
			}

			if (islands.coreIslands.size() != 1)
			{
				splitMe.DestroyAtEndOfFrame();
			}
		}

		if (splitMe.IsAliveAtEndOfFrame())
		{
			Send(event_Sand_CreateCollider{splitMe});
		}
	}

	//
	//
	// All these are helpers functions for sprite splitting
	//
	//

	// splitting

	void SplitFromIsland(const std::vector<int>& island, Entity splitMe, Entity projectile)
	{
		r<Texture> sprite = splitMe.Get<Sprite>()    .source;
		r<Texture> mask   = splitMe.Get<SandSprite>().colliderMask;

		vec2 projectileVel = projectile.Get<Cell>().vel;
		vec2 mid = vec2(mask->Width(), mask->Height()) / 2.f;

		if (island.size() < 15)
		{
			Send(event_Sand_ExplodeToDust{ splitMe, projectile, island });
		}

		else
		{
			auto split = SplitSpriteInTwo(island, splitMe);
			Rigidbody2D& body = splitMe.Get<Rigidbody2D>();

			vec2  v = body.GetVelocity();
			float a = body.GetAngularVelocity();
			float d = 10.f; //body.GetCollider()->GetDensity();

			//Defer([=]()
			//{
				auto [t, s, ss] = split;
				Entity e = CreateEntity().AddAll(Transform2D(t), Sprite(s), SandSprite(ss));
				Send(event_SandAddSprite { e, v, a, d });
			//});
		}
	}

	std::tuple<Transform2D, r<Texture>, r<Texture>> SplitSpriteInTwo(const std::vector<int>& island, Entity toSplit)
	{
		r<Texture> sprite = toSplit.Get<Sprite>().source;
		r<Texture> mask   = toSplit.Get<SandSprite>().colliderMask;

		auto [minX, minY, maxX, maxY] = GetBoundingBoxOfIsland(island, mask->Width());
		vec2 mid = vec2(mask->Width(), mask->Height()) / 2.f;

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

		Transform2D tran = toSplit.Get<Transform2D>();

		vec2 midNew = vec2(minX + maxX + 1, minY + maxY + 1) / 2.f;
		vec2 offset = 2.f * rotate(midNew - mid, tran.rotation);

		tran.position += offset / GetModule<SandWorld>().worldScaleInit;

		return { tran, splitTexture, splitMask };
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