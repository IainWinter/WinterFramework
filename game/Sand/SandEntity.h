#pragma once

#include "Entity.h"
#include "Leveling.h"
#include "Rendering.h"
#include "Physics.h"
#include "Sand/Sand.h"
#include <string>

Entity CreateSandSprite    (const std::string& path, const std::string& collider_mask_path);
Entity CreateTexturedCircle(const std::string& path);