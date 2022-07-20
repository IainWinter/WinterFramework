#pragma once

#include "app/System.h"
#include "Sand/SandEvents.h"
#include "Components/Throwable.h"
#include <algorithm>

struct Sand_System_SplitTiles : System<Sand_System_SplitTiles>
{
	void Init()
	{
		Attach<event_Sand_RemoveCell>();
	}

	void Update()
	{
		std::vector<ToSplit> hits = GetValidHits();

		for (const ToSplit& split : hits)
		{
			SplitSandSprite(split.hit);
		}

		m_hits.clear();
	}

	void on(event_Sand_RemoveCell& e)
	{
		m_hits.insert(ToSplit { e.entity });
	}

private:

	struct ToSplit
	{
		Entity hit;
		ivec2 locationInSprite;
		bool operator==(const ToSplit& t) const { return hit.raw_id() == t.hit.raw_id(); }
	};
	struct ToSplitHash { size_t operator()(const ToSplit& t) const { return t.hit.raw_id(); } };

	std::unordered_set<ToSplit, ToSplitHash> m_hits;

	std::vector<ToSplit> GetValidHits()
	{
		std::vector<ToSplit> valid;

		for (const ToSplit& split : m_hits)
		{
			if (!split.hit.IsAlive()) continue; // I think this is an error

			int cellCount = split.hit.Get<SandSprite>().CellCount();

			if (cellCount == 0)
			{
				split.hit.Destroy();
			}

			else if (cellCount < 15)
			{
				Send(event_Sand_ExplodeToDust{ split.hit });
			}

			else 
			{
				valid.push_back(split);
			}
		}

		return valid;
	}

	void SplitSandSprite(Entity entity)
	{
		auto [transform, drawSprite, sandSprite] = entity.GetAll<Transform2D, Sprite, SandSprite>();
		r<Texture> sprite = drawSprite.source;
		r<Texture> mask = sandSprite.colliderMask;

		Islands islands = GetIslands(sandSprite);

		// Split if there are more than two islands (means there is a gap between two sets of cells in the sprite)
		// If an island contains the core, then its the main peice and should keep the components of the orignal sprite
		// If there are two islands that contain the core, this means it's been split and should explode
		// Any waother island is part of the larger armor, and can be split off by spawning a new entity

		bool split = islands.Count() > 1;

		if (split)
		{
			if (islands.coreIslands.size() > 1) // explode split cores, should tune
			{
				Send(event_SpawnExplosion{ transform.position, 10.f });

				// remove core and continue on to treat this like a sprite without a core
				for (const std::vector<int>& core : islands.coreIslands)
				{
					islands.otherIslands.emplace_back(std::move(core));
				}

				islands.coreIslands.clear();
			}

			// remove all small islands

			for (const std::vector<int>& island : islands.otherIslands)
			{
				if (island.size() < 25)
				{
					Send(event_Sand_ExplodeToDust{ entity, island });
				}
			}

			// sort the set of islands

			std::sort(
				islands.otherIslands.begin(), 
				islands.otherIslands.end(), 
				[](auto& a, auto& b) { return a.size() < b.size(); });

			// remove the largest one, just remove pixels from it

			islands.otherIslands.pop_back();

			for (const std::vector<int>& island : islands.otherIslands)
			{
				SplitFromIsland(island, entity);

				for (const int& index : island)
				{
					sandSprite.pixels.Remove(index);
					sprite->At(index).a = 0;
					mask  ->At(index).a = 0;
				}
			}
		}

		Send(event_Sand_CreateCollider{ entity });
	}

	void SplitFromIsland(const std::vector<int>& island, Entity entity)
	{
		auto split = SplitSpriteInTwo(island, entity);
		Rigidbody2D& body = entity.Get<Rigidbody2D>();

		vec2  v = body.GetVelocity();
		float a = body.GetAngularVelocity();
		float d = body.GetCollider()->GetDensity(); // should test if body has collider, 
													// they should have to because if not they explode
		float cellsStrength = entity.Get<SandSprite>().cellStrength;

		Defer([=]()
		{
			auto [t, s, ss] = split;
			Entity splitEntity = CreateEntity().AddAll(Transform2D(t), Sprite(s), SandSprite(ss));

			splitEntity.Get<SandSprite>()
				.SetCellStrength(cellsStrength);

			vec2  vel = v + safe_normalize(t.position - body.GetPosition()) * 2.f;
			float ang = a + get_randc(wPI);

			if (island.size() < 50)
			{
				splitEntity.Add<SandTurnToDustInTime>(island.size() / 10.f);
			}

			splitEntity.Add<Throwable>();

			SendNow(event_SandAddSprite { splitEntity, vel, ang, d });
		});
	}

	std::tuple<Transform2D, r<Texture>, r<Texture>> SplitSpriteInTwo(const std::vector<int>& island, Entity entity)
	{
		r<Texture> sprite = entity.Get<Sprite>().source;
		r<Texture> mask   = entity.Get<SandSprite>().colliderMask;

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

		Transform2D tran = entity.Get<Transform2D>();

		vec2 midNew = vec2(minX + maxX + 1, minY + maxY + 1) / 2.f;
		vec2 offset = 2.f * rotate(midNew - mid, tran.rotation);

		tran.position += offset / GetModule<SandWorld>().cellsPerMeter;

		return { tran, splitTexture, splitMask };
	}
};