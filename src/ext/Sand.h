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
	r<Texture> colliderMask;

	Texture& Get() { return *colliderMask; }

	SandSprite() = default;
	SandSprite(const Texture& collider) : colliderMask(std::make_shared<Texture>(collider)) {}
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
	float density = 0;
};

// everywhere LevelManager is needed should be inside of a System
// because Systems are owned by whatever level, so it's more contained
// will result in less bugs

struct SandWorld
{
	Entity display;

	vec2 worldScale; // scale of meters to cells
	vec2 screenOffset;
	ivec2 screenSize;

	r<Target> screen;
	Mesh lineProjectiles;

	SandWorld() = default;

	SandWorld(int w, int h, int camScaleX, int camScaleY)
	{
		screen = std::make_shared<Target>(false);
		screen->Add(Target::aColor, w, h, Texture::uINT_32, false);
		//spriteTarget->Add(Target::aDepth, w, h, 1);
		
		lineProjectiles = Mesh(false);
		lineProjectiles.Add<vec2>(Mesh::aPosition, {});
		lineProjectiles.topology = Mesh::dLines;

		Entity e = LevelManager::CurrentLevel()->CreateEntity().AddAll(Transform2D(), lineProjectiles);

		// see Windowing.h
		//screen->Add(Target::aDepth, w, h, Texture::uDEPTH, true);

		worldScale.x = w / camScaleX / 2; // by 2 bc mesh does from -1 to 1
		worldScale.y = h / camScaleY / 2;

		ResizeWorld(w, h, camScaleX, camScaleY);
	}

	void ResizeWorld(int width, int height, int camScaleX, int camScaleY)
	{
		screen->Resize(width, height);
		screenSize = vec2(width, height);
		screenOffset = screenSize / 2;
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
		return *(ivec4*)screen->Get(Target::aColor)->At<int>(pos.x, pos.y);
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
	std::vector<event_SandCellCollision> toSplit; // queue events to not exe a split on every collision, could be multiple per frame on same obj
	std::vector<Entity> tileCache; // for the sprite index in the shader, indexes into this array

	SandCollisionInfoRenderer maskRender;

	void Init()
	{
		Attach<event_WindowResize>();
		Attach<event_SandCellCollision>();
		Attach<event_SandAddSprite>();
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

		e.sprite.Get<Sprite>()    .Get().At(e.hitPosInSprite.x, e.hitPosInSprite.y).a = 0;
		e.sprite.Get<SandSprite>().Get().At(e.hitPosInSprite.x, e.hitPosInSprite.y).a = 0;
		e.sprite.Get<SandSprite>().Get().MarkForUpdate();

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
		transform.scale.x = sprite.Width()  / GetModule<SandWorld>().worldScale.x;
		transform.scale.y = sprite.Height() / GetModule<SandWorld>().worldScale.y;

		// setup collider

		SandSprite& sandSprite = e.entity.Get<SandSprite>();
		sandSprite.density = e.density;

		assert(sandSprite.colliderMask->Channels() == 4 && "collider mask needs to be 32 bit right now, in future should be 8");

		auto polygons = MakePolygonFromField<u32>(
			(u32*)sandSprite.colliderMask->Pixels(),
			sandSprite.colliderMask->Width(),
			sandSprite.colliderMask->Height(),
			[](const u32& color) { return (color & 0xff000000) > 0; }
		);

		Rigidbody2D& body = GetModule<PhysicsWorld>().AddEntity(e.entity);
		body.SetVelocity(e.velocity);
		body.SetAngularVelocity(e.aVelocity);

		for (const std::vector<vec2>& polygon : polygons.first)
		{
			b2PolygonShape shape;
			for (int i = 0; i < polygon.size(); i++)
			{
				shape.m_vertices[i] = _tb(polygon.at(i) * transform.scale);
			}
			shape.Set(shape.m_vertices, polygon.size());

			assert(polygon.size() < 12);
			body.AddCollider(shape, 100.f);
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
		auto [camera, sand, window] = GetModules<Camera, SandWorld, Window>();

		// Sprite update

		for (auto [splitMe, projectile, projectileLocation] : toSplit)
		{
			r<Texture> sprite = splitMe.Get<Sprite>().m_source;
			r<Texture> mask   = splitMe.Get<SandSprite>().colliderMask;
			int length = mask->Width() * mask->Height();

			printf("splitting sprite with length %d\n", length);

			std::vector<flood_fill_cell_state> state = flood_fill_get_states_from_array<u32>(
				(u32*)mask->Pixels(), length, [](const u32& x) { return (x & 0xff000000) > 0; }
			);

			std::vector<std::vector<int>> islands;
			int totalPixelCount = 0;

			for (int seed = 0; seed < length; seed++) // slow could use 'active pixels' cached list
			{
				std::vector<int> island = flood_fill(seed, mask->Width(), mask->Height(), state);
				if (island.size() > 0)
				{
					totalPixelCount += island.size();
					islands.emplace_back(std::move(island));
				}
			}

			Rigidbody2D& body = splitMe.Get<Rigidbody2D>();
			vec2 projectileVel = projectile.Get<Cell>().vel;
			vec2 midOld = vec2(mask->Width(), mask->Height()) / 2.f; // width/height because it's 0-width/height

			body.ApplyForce(projectileVel / 1.f, (vec2(projectileLocation) - midOld) / sand.worldScale);

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
						auto [x, y] = get_xy(index, mask->Width());
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
							auto [x, y] = get_xy(index, sprite->Width());

							vec2 pos = (vec2(x, y) - midOld) / 10.f;
							rotate(pos, splitTransform.rotation);
							pos += splitTransform.position;

							Color color = sprite->At(x, y);
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
					}

					else
					{
						// copy old data to new texture

						Texture splitTexture = Texture(maxX - minX + 1, maxY - minY + 1, Texture::uRGBA, false);
						Texture splitMask    = Texture(maxX - minX + 1, maxY - minY + 1, Texture::uRGBA, false); // could be uR
						
						splitTexture.ClearHost();
						splitMask.ClearHost();

						for (const int& index : island)
						{
							auto [x, y] = get_xy(index, mask->Width());  // this math doesnt need to transform to xy
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
						vec2 offset = 2.f * rotate(midNew - midOld, splitTransform.rotation);
					
						splitTransform.position += offset / sand.worldScale;

						Entity splitOff = LevelManager::CurrentLevel()->CreateEntity()
							.AddAll(splitTransform, Sprite(splitTexture), SandSprite(splitMask));

						vec3 vel;
						if (splitMe.Has<Rigidbody2D>())
						{
							vel.x = splitMe.Get<Rigidbody2D>().GetVelocity().x;
							vel.y = splitMe.Get<Rigidbody2D>().GetVelocity().y;
							vel.z = splitMe.Get<Rigidbody2D>().GetAngularVelocity();
						}

						float density = (float)island.size() / totalPixelCount * splitMe.Get<SandSprite>().density;

						//vec2 pveln = normalize(projectileVel);

						//vel.x += get_randc(.1f) + pveln.x;
						//vel.y += get_randc(.1f) + pveln.y;
						//vel.z += get_randc(.5f);

						SendNow(event_SandAddSprite {splitOff, vec2(vel.x, vel.y), vel.z, density});
					}
				}

				// destroy textures
				sprite->Cleanup();
				mask->Cleanup();

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

		std::vector<std::tuple<vec2, vec2/*, Color*/>> linesToDraw;

		for (auto [e, cell] : QueryWithEntity<Cell>())
		{
			//linesToDraw.push_back(std::make_tuple(
			//	cell.pos / sand.worldScale, 
			//	cell.pos + cell.vel * Time::DeltaTime() / cell.dampen / sand.worldScale/*, 
			//	cell.color*/
			//));

			if (length(cell.vel / cell.dampen * Time::DeltaTime()) > 1)
			{
				linesToDraw.push_back(DrawLine(sand, collisionMaskRender, e, cell));
			}

		//	else
		//	{
		//		if (sand.OnScreen(cell.pos))
		//		{
		//			//sand.DrawPixel(color, cell.pos, cell.color);
		//		}

		//		cell.pos += cell.vel * Time::DeltaTime();
		//	}
		}

		if (linesToDraw.size() > 0)
		{
			sand.lineProjectiles.Get(Mesh::aPosition)->Set(linesToDraw.size()*2, linesToDraw.data());

			printf("%d %p\n", linesToDraw.size(), sand.lineProjectiles.Get(Mesh::aPosition)->Data());

			linesToDraw.clear();
		}
	}

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

				if (tileEntity.Id() != e.Get<CellProjectile>().owner)
				{
					SendNow(event_SandCellCollision { tileEntity, e, positionInSprite }); // should defer
				}

				vec2& v = e.Get<Cell>().vel;
				v = normalize(v + get_rand(10.f, 10.f)) * length(v);
			}

			//sand.DrawPixel(display, raster, cell.color);
			current += delta;
			cell.pos = current;
		}

		return { origin / sand.worldScale, current / sand.worldScale };
	}
};
