#include "Entry.h"
#include "Common.h"
#include "ext/Time.h"

#include "LevelSystem.h"

void setup()
{
	// create all the levels / load them from a file
}

int score = 0;

bool loop()
{
	Time::UpdateTime();
	if (Time::DeltaTime() > 2) score += get_rand(100);

	// update level system based on score?

	return true;
}