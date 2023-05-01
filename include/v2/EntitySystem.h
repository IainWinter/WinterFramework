#pragma once

#include <vector>
#include <unordered_map>
#include <tuple>
#include <assert.h>
#include <utility>

// Goal of this system is to allow the game state to just be a struct and have the
// data/entities opt into the tracking system themselves.

// A generic entity
// Can bind to variables in concrete entities to mimic runtime composition.
// On the first call to Id, an id is generated which maps the entity to 
// its this pointer even if copied or moved.
class Entity_
{
public:
    // Return a component or assert if it isn't bound
    template<typename _t>
    _t& Get();

    // Return a component or nullptr if it isn't bound
    template<typename _t>
    _t* TryGet();

    // Assign or get an ID so other systems have a 
    // way to look this entity up even if its memory is moved
    // see gResolveEntity
    int Id();

public:
    Entity_();
    ~Entity_();
    Entity_(const Entity_& other);
    Entity_(Entity_&& other) noexcept;
    Entity_& operator=(const Entity_& other);
    Entity_& operator=(Entity_&& other) noexcept;

private:
    void copy_from(const Entity_& other);
    void move_from(Entity_&& other) noexcept;
    void destroy();

protected:
    // Call from a concrete class to bind data
    template<typename... _t>
    void _bind(_t&... ptr);

    // Called when bound data is needed, override to bind custom data
    virtual void Bind();

private:
    // Map each component type to a sequential integer
    template<typename _t>
    int GetComponentId();

    // If data hasn't already been bound, call Bind
    void AttemptBind();

private:
    struct EntityBinder {
        std::unordered_map<int, void*> values;
    };

    int id;
    bool attemptedBind;
    EntityBinder* bounded;

    static int s_nextComponentId;
};

// I want to be able to store entities without pointers to get rid of the, is-alive problem that
// stems from the fact that in most systems entities can be dead or alive. This is also an issue with raw pointers.
// To fix this, entities need to have an id that maps them to their memory address. This class acts as that map and
// is made as a singleton because these ids should act just like pointers, but with 1 more indirection to mask the fact
// that the underlying entity can move in memory. Basically just Java refs
class EntityResolver
{
public:
    // Generate a new Id and map it to the pointer
    int Map(Entity_* ptr);

    // Update an id to a new pointer
    void Update(int id, Entity_* ptr);

    // Get a pointer or nullptr from an id
    Entity_* Get(int id);

    // Remove a mapping
    void Remove(int id);

private:
    std::unordered_map<int, Entity_*> entities;
    int nextId = 0;
};

// Get an entity pointer or nullptr from an id
Entity_* gResolveEntity(int id);

//
//  Template implementation
//

template<typename _t>
_t& Entity_::Get()
{
    int tid = GetComponentId<_t>();

    AttemptBind();

    assert(bounded 
        && bounded->values.count(tid) != 0 
        && "Need to first bind a value to get it");
        
    return *(_t*)bounded->values.at(tid);
}

template<typename _t>
_t* Entity_::TryGet()
{
    int tid = GetComponentId<_t>();

    AttemptBind();

    if (!bounded || bounded->values.count(tid) == 0)
        return nullptr;

    return (_t*)bounded->values.at(tid);
}

template<typename... _t>
void Entity_::_bind(_t&... ptr)
{
    // opt-in through lazy init
    if (!bounded)
        bounded = new EntityBinder();

    (bounded->values.emplace(GetComponentId<_t>(), &ptr), ...);
}

template<typename _t>
int Entity_::GetComponentId()
{
    static int id = s_nextComponentId++;
    return id;
}