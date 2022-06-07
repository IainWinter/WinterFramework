#include "Common.h"
#include "ext/Time.h"
#include <math.h>
#include <functional>
#include "LevelSystem.h"
#include <vector>

#include "EngineLoop.h"

struct SpawnerUpdateSystem : System<SpawnerUpdateSystem>
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

struct Regolith : EngineLoop
{
	void _Init()
	{
		Window& window = m_app.GetModule<Window>();
		window.SetTitle("Regolith");
		window.Resize(1280, 720);

		r<Level> level = LevelManager::CurrentLevel();
		level->AddSystem(SpawnerUpdateSystem());
	}
};

void setup()
{
	RunEngineLoop<Regolith>();
}




// rip

//struct Phase
//{
//	float StartDifficulty;
//	float EndDifficulty;
//	int EndScore;
//
//	Phase(float startDifficulty, float endDifficulty, int endScore)
//	{
//		StartDifficulty = startDifficulty;
//		EndDifficulty = endDifficulty;
//		EndScore = endScore;
//	}
//
//	float GetDifficulty(int relScore) 
//	{
//		// invalid deal with later ---> next step
//		// difficulty = (score < 0.5 ? 8 * score * score * score * score : 1 - pow(-2 * score + 2, 4) / 2);
//		float RelScore = relScore;
//		return lerp(StartDifficulty, EndDifficulty, RelScore);
//	}
//};
//
//struct Level
//{
//	// startpoint and endpoint are difficulty measurements
//	std::vector<Phase> phases;
//
//public: 
//	void AddPhase(float startDifficulty, float endDifficulty, int scoreLength)
//	{
//		assert(scoreLength > 0);
//
//		int currentScore = 0;
//		if (phases.size() > 0)
//		{
//			currentScore = phases.back().EndScore;
//		}
//
//		phases.push_back(Phase(startDifficulty, endDifficulty, currentScore + scoreLength));
//	}
//
//	Phase& GetPhase(int currentScore)
//	{
//		assert(phases.size() > 0);
//
//		for (int i = 0; i < phases.size(); i++)
//		{
//			if (currentScore < phases[i].EndScore)
//			{
//				return phases[i];
//			}
//		}
//
//		return phases.back();
//	}
//
//	void SpawnNext(int currentScore)
//	{
//		Phase& currentPhase = GetPhase(currentScore);
//	}
//
//	void SpawnFighter(float x, float y) {} 
//	void SpawnBomb(float x, float y) {}
//	void SpawnStation(float x, float y) {}
//	void SpawnBase(float x, float y) {}
//};
//
//void setup()
//{
//	Level currentLevel;
//
//	// create all the levels / load them from a file
//
//	// arbitrary score thresholds
//	// start at phase 1
//	// if score = 200* go to phase 2
//	// if score = 1000*  go to phase 3
//	// if score = 5000* go to phase 4
//	// if score = 15000* go to phase 5(boss)
//
//	currentLevel.AddPhase(0, 1, 200);
//	currentLevel.AddPhase(0, 1, 1000-200);
//	currentLevel.AddPhase(0, 1, 5000-1000);
//	currentLevel.AddPhase(0, 1, 15000-5000);
//
//	// Test Code
//	currentLevel.SpawnNext(100);
//	currentLevel.SpawnNext(1580);
//	currentLevel.SpawnNext(1000000);
//}
//
//
//
//bool loop()
//{
//
//
//}
