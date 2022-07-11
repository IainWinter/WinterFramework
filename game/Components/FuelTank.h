#pragma once

#include "Sand/DiscreteSandWorld.h"
#include <deque>

struct LaserTank
{
	int _pad;
};

struct FuelTank
{
	SimpleSandWorld cells;
	const char* mask;

	std::vector<std::pair<int, int>> inlet;
	std::vector<std::pair<int, int>> outlet;
	
	std::deque<Color> feed;

	bool openOutlet = false;
	std::function<void()> onEat;

	float fuelTankUpdateTimer = 0.f;
	float fuelTankUpdateRate  = 1 / 60.f;
};