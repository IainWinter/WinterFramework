#include "ext/serial/SerializeEntityWorld.h"

void RegisterMetaTypes()
{
    meta::describe<EntityData>()
        .name("EntityData")
        .member<&EntityData::id>("id")
        .member<&EntityData::components>("components");
}

EntityData GetEntityData(entt::registry& reg, entt::entity e)
{
    EntityData data;
    data.id = (u32)e;

    for (auto&& [component, store] : reg.storage())
    {
        if (store.contains(e))
        {
            meta::type* type = meta::from_entt(component);
            void* ptr = store.get(e);

            if (!type)
            {
                log_io("e~Failed to serialize type, no type information. %s", entt::type_id(component).name().data());
                continue;
            }

            data.components.push_back(meta::any(type, ptr));
        }
    }

    return data;
}

void WriteWorld(World* world, meta::serial_writer& writer)
{
    std::vector<EntityData> entities;

    entt::registry& reg = world->GetEntityWorld().entt();
    reg.each([&](entt::entity e)
    {
        entities.push_back(GetEntityData(reg, e));
    });

    writer.write(entities);
}

void ReadWorld(World* world, meta::serial_reader& reader)
{
    std::vector<EntityData> entities;
    reader.read(entities);

    entt::registry& reg = world->GetEntityWorld().entt();
    reg = {}; // clear all memory to reallocate here

    for (EntityData& data : entities)
    {
        entt::entity entity = reg.create((entt::entity)data.id);

        for (meta::any& any : data.components)
        {
            any.type()->ping(&reg, REG_STORAGE);
            reg.storage(any.type_id())->emplace(entity, any.data());
        }
    }
}