#include "v2/EntitySystem.h"
#include "Log.h"

int v2Entity::s_nextComponentId = 0;

static EntityResolver resolver;

v2Entity::v2Entity()
    : id            (0)
    , attemptedBind (false)
    , bounded       (nullptr)
{}

v2Entity::~v2Entity()
{
    destroy();
}

v2Entity::v2Entity(const v2Entity& other)
    : id            (0)
    , attemptedBind (false)
    , bounded       (nullptr)
{
    copy_from(other);
}

v2Entity::v2Entity(v2Entity&& other) noexcept
    : id            (0)
    , attemptedBind (false)
    , bounded       (nullptr)
{
    move_from(std::forward<v2Entity>(other));
}

v2Entity& v2Entity::operator=(const v2Entity& other)
{
    copy_from(other);
    return *this;
}

v2Entity& v2Entity::operator=(v2Entity&& other) noexcept
{
    move_from(std::forward<v2Entity>(other));
    return *this;
}

void v2Entity::copy_from(const v2Entity& other)
{
    // need to destroy the data and not copy anything
    destroy();

    // don't assign an id, let the concrete class decide if it wants one
    id = 0;
    
    // reset bind info so new memory locations get bound
    bounded = nullptr;
    attemptedBind = false;
}

void v2Entity::move_from(v2Entity&& other) noexcept
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

void v2Entity::destroy()
{
    delete bounded;
    bounded = nullptr;

    if (id > 0)
        resolver.Remove(id);
}

int v2Entity::Id()
{
    if (id > 0)
        return id;

    id = resolver.Map(this);

    return id;
}

int v2Entity::Id() const
{
    return id;
}

std::unordered_set<size_t> v2Entity::GetArchetype()
{
    AttemptBind();

    if (!bounded)
        return {};

 //   return bounded->archetype;

    std::unordered_set<size_t> archetype;

    for (auto [component_id, _] : bounded->values)
        archetype.insert(component_id);

    return archetype;
}

bool v2Entity::operator==(const v2Entity& other) const 
{
    if (Id() == 0 || other.Id() == 0)
        return false;

    return Id() == other.Id(); 
}

bool v2Entity::operator!=(const v2Entity& other) const 
{
    if (Id() == 0 || other.Id() == 0)
        return true;

    return Id() != other.Id();
}

void v2Entity::Bind() {}

void v2Entity::AttemptBind()
{
    if (attemptedBind)
        return;

    attemptedBind = true;

    Bind();
}

int EntityResolver::Map(v2Entity* ptr)
{
    int id = ++nextId;
    entities[id] = ptr;

    return id;
}

void EntityResolver::Update(int id, v2Entity* ptr)
{
    if (entities.count(id) == 0)
        throw nullptr;

    entities[id] = ptr;
}

v2Entity* EntityResolver::Get(int id)
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

v2Entity* gResolveEntity(int id)
{
    return resolver.Get(id);
}
