#include "v2/EntitySystem.h"

int Entity_::s_nextComponentId = 0;

static EntityResolver resolver;

Entity_::Entity_()
    : id            (0)
    , attemptedBind (false)
    , bounded       (nullptr)
{}

Entity_::~Entity_()
{
    destroy();
}

Entity_::Entity_(const Entity_& other)
{
    copy_from(other);
}

Entity_::Entity_(Entity_&& other) noexcept
{
    move_from(std::forward<Entity_>(other));
}

Entity_& Entity_::operator=(const Entity_& other)
{
    copy_from(other);
    return *this;
}

Entity_& Entity_::operator=(Entity_&& other) noexcept
{
    move_from(std::forward<Entity_>(other));
    return *this;
}

void Entity_::copy_from(const Entity_& other)
{
    // need to destroy the data and not copy anything
    destroy();

    // don't assign an id, let the concrete class decide if it wants one
    id = 0;
    
    // reset bind info so new memory locations get bound
    bounded = nullptr;
    attemptedBind = false;
}

void Entity_::move_from(Entity_&& other) noexcept
{
    // can't move because the location of data has changed
    copy_from(other);

    // update id to point to this new memory
    if (other.id > 0)
    {
        resolver.Update(other.id, this);
        id = other.id;
        other.id = 0;
    }
}

void Entity_::destroy()
{
    delete bounded;
}

int Entity_::Id()
{
    if (id > 0)
        return id;

    id = resolver.Map(this);

    return id;
}

void Entity_::Bind()
{}

void Entity_::AttemptBind()
{
    if (attemptedBind)
        return;

    attemptedBind = true;

    Bind();
}

int EntityResolver::Map(Entity_* ptr)
{
    int id = ++nextId;
    entities[id] = ptr;
    return id;
}

void EntityResolver::Update(int id, Entity_* ptr)
{
    assert(entities.count(id) > 0 && "Entity resolver has not mapped id");
    entities[id] = ptr;
}

Entity_* EntityResolver::Get(int id)
{
    auto itr = entities.find(id);

    if (itr == entities.end())
        return nullptr;

    return itr->second;
}

void EntityResolver::Remove(int id)
{
    auto itr = entities.find(id);

    if (itr == entities.end())
        return;

    entities.erase(itr);
}

Entity_* gResolveEntity(int id)
{
    return resolver.Get(id);
}