#pragma once

#include "app/System.h"
#include "Sand/Sand.h"

struct InitGame_System : SystemBase
{
	void Init()
	{
		vec2 screenSize = GetWindow().Dimensions();
		vec2 cameraSize = vec2(16 * 2, 9 * 2);

		int cellsPerMeter = 18;

		CreateEntity().Add<SandWorld>(cellsPerMeter, cameraSize);
		CreateEntity().Add<Camera>(0, 0, cameraSize.x, cameraSize.y);

		CoordTranslation& coords = CreateEntity().Add<CoordTranslation>();
		coords.ScreenToWorld = cameraSize;
		coords.CellsToMeters = 1.f / cellsPerMeter;
	}
};