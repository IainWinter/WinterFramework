#pragma once

#include "Entity.h"
#include "ext/rendering/Camera.h"

void render_init();
void render_dnit();

void render(const Camera& camera, EntityWorld& world);