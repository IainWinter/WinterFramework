#include "Entry.h"
#include "Common.h"
#include "ext/Time.h"

#include "LevelSystem.h"

void setup()
{
	// create all the levels / load them from a file
}

int score = 0;

float acc = 0.f;
bool loop()
{
	Time::UpdateTime();
	acc += Time::DeltaTime();
	if (acc > 2)
	{
		acc = 0;
		score += get_rand(100);
		printf("current score %d\n", score);
	}

	// update level system based on score?

	return true;
}