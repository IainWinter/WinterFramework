#pragma once

#include "Entity.h"
#include "Windowing.h"
#include "Rendering.h"
#include "Physics.h"

void global_init(Window& window, EventQueue& queue, PhysicsWorld& physics, EntityWorld& world);
void global_dnit(Window& window, EntityWorld& world);

void tick_pre(Window& window);
void tick_frame(Window& window, EventQueue& event, EntityWorld& world, PhysicsWorld& physics);

bool is_running();