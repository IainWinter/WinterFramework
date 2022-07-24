#pragma once

#include "Entity.h"
#include "app/System.h"
#include "Rendering.h"
#include "Physics.h"
#include "Sand/Sand.h"
#include <string>

Entity CreateSandSprite    (r<World> world, const std::string& path, const std::string& collider_mask_path);
Entity CreateTexturedCircle(r<World> world, const std::string& path);