#pragma once

#include "Entity.h"
#include "Windowing.h"
#include "Rendering.h"
#include "Physics.h"
#include "Audio.h"

void global_init(Window& window, EventQueue& queue, PhysicsWorld& physics, EntityWorld& world, AudioWorld& audio);
void global_dnit(Window& window, EntityWorld& world, AudioWorld& audio);

void tick_pre(Window& window);
void tick_frame(Window& window, EventQueue& event, PhysicsWorld& physics, EntityWorld& world, AudioWorld& audio);

bool is_running();