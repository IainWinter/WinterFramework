#pragma once

#include "Leveling.h"
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

		if (e.entity.Has<SandHealable>())
		{
			SandHealable& healable = e.entity.Get<SandHealable>();
			bool isCore = std::find(mask.core.begin(), mask.core.end(), e.index) != mask.core.end();

			if (isCore) healable.core .push_back (e.index);
			else        healable.shell.push_back(e.index);
		}

		sprite.Get().At(e.index).a = 0;
		mask  .Get().At(e.index).a = 0;
		
		mask.cellCount -= 1;
	}

	void on(event_Sand_HealCell& e)
	{
		auto [sprite, mask, healable] = e.entity.GetAll<Sprite, SandSprite, SandHealable>();
		int index = e.index;
		
		if (index == -1)
		{
			ivec2 dim = sprite.source->Dimensions();
			int cx = dim.x / 2;
			int cy = dim.y / 2;

			float minDist = FLT_MAX;
			for (const int& i : healable.shell)
			{
				auto [x, y] = get_xy(i, dim.x);
				int dx = cx - x;
				int dy = cy - y;
				float dist = sqrt(dx*dx + dy*dy);
						
				if (dist < minDist)
				{
					minDist = dist;
					index = i;
				}
			}
		}

		Color& sc = sprite.Get().At(index);
		Color& mc = mask  .Get().At(index);

		if (sc.as_u32 != 0) { sc.a = 255; }
		if (mc.as_u32 != 0) { mc.a = 255; mask.cellCount += 1; }

		auto itrCore  = std::find(healable.core .begin(), healable.core .end(), index);
		auto itrShell = std::find(healable.shell.begin(), healable.shell.end(), index);

		bool isCore  = itrCore  != healable.core .end();
		bool isShell = itrShell != healable.shell.end();

		if (isCore)  healable.core .erase(itrCore);
		if (isShell) healable.shell.erase(itrShell);
	}
};