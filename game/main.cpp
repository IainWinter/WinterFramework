#include "Entry.h"
#include "Common.h"
#include "ext/Time.h"

#include "LevelSystem.h"
#include <vector>


struct Phase
{
	float StartDifficulty;
	float EndDifficulty;
	int EndScore;

	Phase(float startDifficulty, float endDifficulty, int endScore)
	{
		StartDifficulty = startDifficulty;
		EndDifficulty = endDifficulty;
		EndScore = endScore;
	}

	float GetDifficulty(int currentScore) 
	{
		// invalid deal with later ---> next step
		float RelScore = currentScore;
		return lerp(StartDifficulty, EndDifficulty, RelScore);
	}
};

struct Level
{
	// startpoint and endpoint are difficulty measurements
	std::vector<Phase> phases;


public: 
	void AddPhase(float startDifficulty, float endDifficulty, int scoreLength)
	{
		assert(scoreLength > 0);

		int currentScore = 0;
		if (phases.size() > 0)
		{
			currentScore = phases.back().EndScore;
		}

		phases.push_back(Phase(startDifficulty, endDifficulty, currentScore + scoreLength));
	}

	Phase& GetPhase(int currentScore)
	{
		assert(phases.size() > 0);

		for (int i = 0; i < phases.size(); i++)
		{
			if (currentScore < phases[i].EndScore)
			{
				return phases[i];
			}
		}

		return phases.back();
	}

	void SpawnNext(int currentScore)
	{

		Phase& currentPhase = GetPhase(currentScore);

	}

	void SpawnFighter(float x, float y) {} 
	void SpawnBomb(float x, float y) {}
	void SpawnStation(float x, float y) {}
	void SpawnBase(float x, float y) {}
};

void setup()
{
	Level currentLevel;

	// create all the levels / load them from a file

	// arbitrary score thresholds
	// start at phase 1
	// if score = 200* go to phase 2
	// if score = 1000*  go to phase 3
	// if score = 5000* go to phase 4
	// if score = 15000* go to phase 5(boss)

	currentLevel.AddPhase(0, 1, 200);
	currentLevel.AddPhase(0, 1, 1000-200);
	currentLevel.AddPhase(0, 1, 5000-1000);
	currentLevel.AddPhase(0, 1, 15000-5000);

	// Test Code
	currentLevel.SpawnNext(100);
	currentLevel.SpawnNext(1580);
	currentLevel.SpawnNext(1000000);

}

int score = 0;
float difficulty = 0;

float acc = 0.f;

float playerstatus = 0.f;
float enemystatus = 0.f;
float enemytype = 0.f;

float fighterspawnchance = 0.f;
float bomberspawnchance = 0.f;
float stationspawnchance = 0.f;

bool loop()
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

		difficulty = (score < 0.5 ? 8 * score * score * score * score : 1 - pow(-2 * score + 2, 4) / 2);

		// Each enemy type spawn rate
		fighterspawnchance = 0;
		bomberspawnchance = 0;
		stationspawnchance = 0;

	}

	if (playerstatus == 0)
	{
		printf("Final Score %d\n", score);
		return false;
	}

	// update level system based on score?

	
	return true;
}
