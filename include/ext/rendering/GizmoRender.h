#pragma once

#include "Entity.h"
#include "ext/rendering/Camera.h"

void Gizmo_RenderPhysicsColliders(const Camera& camera, EntityWorld& world);
void Gizmo_RenderCamearFrustum   (const Camera& camera, EntityWorld& world);