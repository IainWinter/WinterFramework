#pragma once

#include "Leveling.h"
#include "Systems/CreateCollider.h"

inline void AddSandSystemsToLevel(r<Level> level)
{
	level->AddSystem(Sand_System_CreateCollider());
}