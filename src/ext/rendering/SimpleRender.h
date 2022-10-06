#pragma once

#include "Entity.h"
#include "Rendering.h"
#include "ext/rendering/Camera.h"
#include "ext/rendering/BatchSpriteRenderer.h"
#include "ext/rendering/BatchLineRenderer.h"
#include "ext/rendering/Particle.h"
#include "ext/rendering/Model.h"
#include "ext/rendering/Sprite.h"

enum ShowEntityIdMode
{
	NONE,
	MIX,
	ONLY
};

struct event_ShowEntityIds
{
	ShowEntityIdMode mode;
};

void InitRendering();

void SetDrawEntityDebugMode(ShowEntityIdMode mode);

void RenderMeshes  (const Camera& camera, EntityWorld& world);
void RenderSprites (const Camera& camera, EntityWorld& world);
void RenderLines   (const Camera& camera, EntityWorld& world);

r<Mesh> GetQuadMesh2D();
Mesh& InitQuadMesh2D(Mesh& mesh);

r<ShaderProgram> GetProgram_Sprite();
