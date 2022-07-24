#pragma once

#include "Sand/SandSystems.h"

#include "Sand/Systems/CreateCollider.h"
#include "Sand/Systems/CollideProjectile.h"
#include "Sand/Systems/ExplodeToDust.h"
#include "Sand/Systems/SplitTiles.h"
#include "Sand/Systems/UpdateLineProjectileMesh.h"
#include "Sand/Systems/RemoveCellFromSprite.h"

std::vector<SystemBase*> sand_ids;

void CreateSandSystems(World* level)
{
	sand_ids = {
		level->CreateSystem(Sand_System_Update()),
		level->CreateSystem(Sand_System_UpdateLineProjectileMesh()),
		level->CreateSystem(Sand_System_SplitTiles()),

		// reactive systems

		level->CreateSystem(Sand_System_ExplodeToDust()),
		level->CreateSystem(Sand_System_CollideProjectile()),
		level->CreateSystem(Sand_System_CreateCollider()),
		level->CreateSystem(Sand_System_RemoveCellsFromSprite())
	};
}

void DestroySandSystems(r<World> level)
{
	for (SystemBase*& s : sand_ids)
	{
		level->DestroySystem(s);
	}
	
	sand_ids = {};
}