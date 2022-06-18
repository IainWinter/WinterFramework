#pragma once

#include "Leveling.h"
#include "Sand/Systems/CreateCollider.h"
#include "Sand/Systems/CollideProjectile.h"
#include "Sand/Systems/DrawSandSprites.h"
#include "Sand/Systems/ExplodeToDust.h"
#include "Sand/Systems/SplitTiles.h"
#include "Sand/Systems/UpdateLineProjectileMesh.h"

inline void AddSandSystemsToLevel(r<Level> level)
{
	level->AddSystem(Sand_System_Update());
	
	level->AddSystem(Sand_System_UpdateLineProjectileMesh());
	level->AddSystem(Sand_System_DrawSandSprites());
	//level->AddSystem(Sand_System_SplitTiles());


	// reactive systems

	level->AddSystem(Sand_System_ExplodeToDust());
	level->AddSystem(Sand_System_CollideProjectile());
	level->AddSystem(Sand_System_CreateCollider());
}