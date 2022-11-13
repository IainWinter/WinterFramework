#pragma once

#include "ext/serial/serial.h"
#include "Entity.h"
#include "app/System.h"

struct EntityData
{
    u32 id;
    std::vector<meta::any> components;
};

void RegisterMetaTypes();

EntityData GetEntityData(entt::registry& reg, entt::entity e);
void WriteWorld(World* world, meta::serial_writer& writer);
void ReadWorld(World* world, meta::serial_reader& reader);