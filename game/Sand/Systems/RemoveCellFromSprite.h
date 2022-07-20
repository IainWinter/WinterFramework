#pragma once

#include "app/System.h"
#include "Sand/SandEvents.h"

struct Sand_System_RemoveCellsFromSprite : System<Sand_System_RemoveCellsFromSprite>
{
	void Init()
	{
		Attach<event_Sand_RemoveCell>();
		Attach<event_Sand_HealCell>();
	}

	void on(event_Sand_RemoveCell& e)
	{
		auto [sprite, mask] = e.entity.GetAll<Sprite, SandSprite>();
		bool isCore = std::find(mask.initalCore.begin(), mask.initalCore.end(), e.index) != mask.initalCore.end();
		
		if (e.entity.Has<SandHealable>())
		{
			SandHealable& healable = e.entity.Get<SandHealable>();

			if (isCore) healable.removedFromCore .push_back(e.index);
			else        healable.removedFromShell.push_back(e.index);
		}

		mask.pixels.Remove(e.index);
		
		sprite.Get().At(e.index).a = 0;
		mask  .Get().At(e.index).a = 0;
	}

	void on(event_Sand_HealCell& e)
	{
		auto [sprite, mask, healable] = e.entity.GetAll<Sprite, SandSprite, SandHealable>();
		int index = e.index;
		
		if (index == -1) // heal closest to center
		{
			ivec2 dim = sprite.source->Dimensions();
			int cx = dim.x / 2;
			int cy = dim.y / 2;

			float minDist = FLT_MAX;

			std::vector<int>& candidates = e.healCore ? healable.removedFromCore : healable.removedFromShell;
			for (const int& i : candidates)
			{
				auto [x, y] = get_xy(i, dim.x);
				int dx = cx - x;
				int dy = cy - y;
				float dist = sqrtf(dx*dx + dy*dy);
						
				if (dist < minDist)
				{
					minDist = dist;
					index = i;
				}
			}
		}

		Color& sc = sprite.Get().At(index);
		Color& mc = mask  .Get().At(index);

		if (sc.as_u32 != 0) sc.a = e.healCore ? 254 : 255; // this doesnt work for some reason
		if (mc.as_u32 != 0) mc.a = 255;

		// create Heal function

		if (e.healCore)
		{
			auto itr = std::find(healable.removedFromCore.begin(), healable.removedFromCore.end(), index);
			if (itr != healable.removedFromCore.end()) {
				healable.removedFromCore.erase(itr);
			}
		}

		else
		{
			auto itr = std::find(healable.removedFromShell.begin(), healable.removedFromShell.end(), index);
			if (itr != healable.removedFromShell.end()) {
				healable.removedFromShell.erase(itr);
			}
		}
		
		mask.pixels.Add(index, e.healCore);
	}
};