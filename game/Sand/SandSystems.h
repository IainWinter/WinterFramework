#pragma once

#include "Leveling.h"
#include "Sand/Systems/CreateCollider.h"
#include "Sand/Systems/ExplodeToDust.h"

inline void AddSandSystemsToLevel(r<Level> level)
{
	level->AddSystem(Sand_System_CreateCollider());
	level->AddSystem(Sand_System_ExplodeToDust());
	level->AddSystem(Sand_System_Update());
}