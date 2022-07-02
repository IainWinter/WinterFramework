#pragma once

#include "Leveling.h"
#include "ext/Time.h"

struct SpawningBounds
{
	vec2 min;
	vec2 max;
};

// WorldSize is a scale from -x to x which originates at the center of the screen (0,0)
// Example: if WorldSize = (10, 10) then the bounds of the screen go from (-5,-5) to (5,5)
inline std::vector<SpawningBounds> GetSpawningBounds(vec2 worldSize, vec2 padding, int numberOfZonesPerSide)
{
	using namespace std;
	vector <SpawningBounds> bounds;
	vec2 directions[4] = {vec2(1,0), vec2(0,1), vec2(-1,0), vec2(0,-1)};
	for (vec2& dir : directions) 
	{
		// do work here. to add to vector use ".push_back(bound)"
		// Order
		// Right Zone
		// Top Zone
		// Left Zone
		// Bottom Zone

		SpawningBounds bound;
		bound.min = dir * worldSize;
		bound.max = dir * worldSize + padding;
		bounds.push_back(bound);

		
	}

	return bounds;
}

struct System_SpawnerUpdate : System<System_SpawnerUpdate>
{
	int score = 0;
	float difficulty = 0;

	float acc = 0.f;

	float playerstatus = 0.f;
	float enemystatus = 0.f;
	float enemytype = 0.f;

	float fighterspawnchance = 0.f;
	float bomberspawnchance = 0.f;
	float stationspawnchance = 0.f;

	void Update()
	{
		Time::UpdateTime();
		acc += Time::DeltaTime();
		//return x < 0.5 ? 8 * x * x * x * x : 1 - pow(-2 * x + 2, 4) / 2; ---> implement function into score
		playerstatus = 10;

		if (playerstatus != 0)
		{
			// 3 enemy types (1 = easy, 2 = mid, 3 = hard)
			enemytype = get_rand(3) + 1;

			// if 0, destroyed    if 1, still alive
			enemystatus = get_rand(2);

			// if 0, dead       if 1-9 alive
			playerstatus = get_rand(10);

			if (enemystatus == 0)
			{
				if (enemytype == 1)
				{
					score += 10;
				}

				if (enemytype == 2)
				{
					score += 50;
				}

				if (enemytype == 3)
				{
					score += 100;
				}
			}
			printf("Current Score %d\n", score);

			float difficulty = 0;

			// Each enemy type spawn rate
			fighterspawnchance = 0;
			bomberspawnchance = 0;
			stationspawnchance = 0;
		}

		if (playerstatus == 0)
		{
			printf("Final Score %d\n", score);
		}

		// update level system based on score?

		// Outline

		// Difficulty is only dependent on score value
		// Difficulty = function(currentscore)
		// getDifficulty(currentscore) is called to calculate difficulty -> returns number between 0 and 1
		// Difficulty is between 0 and 1 ---> for score = 0, difficulty starts at 0 and goes up with score

		std::function<float(int)> getfighterspawnchance = [](int difficulty) {return exp(1 - difficulty) / exp(1); };

		// ARBITRARY VALUES

		float fighterspawnchance = getfighterspawnchance(difficulty);
		float bomberspawnchance = (1 - fighterspawnchance) * (0.4 / 0.6321);
		float stationspawnchance = 1 - fighterspawnchance - bomberspawnchance;
		// all 3 values should add up to 1

		// weighted random number generator to determine which enemy spawns
		float randnum = get_rand(1.f);

		if (randnum <= fighterspawnchance)
		{
			// spawn fighter
		}

		if (randnum > fighterspawnchance && randnum <= fighterspawnchance + bomberspawnchance)
		{
			// spawn bomber
		}

		if (randnum > fighterspawnchance + bomberspawnchance + stationspawnchance)
		{
			// spawn station
		}

		int maxfighterspawn = 10;
		int maxbomberspawn = 6;
		int maxstationspawn = 3;

		// enemynumberchance
		// if fighter selected (10 spawners)
			// spawnchance = 0.3 + (0.7)difficulty ---> for each spawner

		// if bomb selected (6 spawners)
			// spawnchance = 0.2 + (0.8)difficulty ---> for each spawner

		// if station selected (3 spawners)
			// spawnchance = 0.1 + (0.9)difficulty ---> for each spawner

		// implement chance values into spawnenemy functions
	}
};