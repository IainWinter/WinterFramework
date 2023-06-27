#pragma once

#include <unordered_map>
#include <unordered_set>

// Goal of this system is to allow the game state to just be a struct and have the
// data/entities opt into the tracking system themselves.

// A generic entity
// Can bind to variables in concrete entities to mimic runtime composition.
// On the first call to Id, an id is generated which maps the entity to 
// its this pointer even if copied or moved.
class v2Entity
{
public:
    // Return a component or assert if it isn't bound
    template<typename _t>
    _t& Get();

    // Return a component or nullptr if it isn't bound
    template<typename _t>
    _t* TryGet();

    // Return true if a component is bound
    template<typename _t>
    bool Has();

    // Assign or get an ID so other systems have a 
    // way to look this entity up even if its memory is moved
    // see gResolveEntity
    int Id();

    // Get the previously assigned ID of this entity
    int Id() const;

    // Generate a list of the component ids which are added through _bind
    std::unordered_set<size_t> GetArchetype();

public:
    bool operator==(const v2Entity& other) const;
    bool operator!=(const v2Entity& other) const;

public:
    v2Entity();
    virtual ~v2Entity();
    v2Entity(const v2Entity& other);
    v2Entity(v2Entity&& other) noexcept;
    v2Entity& operator=(const v2Entity& other);
    v2Entity& operator=(v2Entity&& other) noexcept;

private:
    void copy_from(const v2Entity& other);
    void move_from(v2Entity&& other) noexcept;
    void destroy();

protected:
    // Call from a concrete class to bind data
    template<typename... _t>
    void _bind(_t&... ptr);

    // Called when bound data is needed, override to bind custom data
    virtual void Bind();

public:
    // Called when entity is removed form an entity_list
    virtual void Destroy();

private:
    // Map each component type to a sequential integer
    template<typename _t>
    size_t GetComponentId();

    // If data hasn't already been bound, call Bind
    void AttemptBind();

private:
    struct EntityBinder {
        std::unordered_map<size_t, void*> values;
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
    int Map(v2Entity* ptr);

    // Update an id to a new pointer
    void Update(int id, v2Entity* ptr);

    // Get a pointer or nullptr from an id
    v2Entity* Get(int id);

    // Remove a mapping
    void Remove(int id);

private:
    std::unordered_map<int, v2Entity*> entities;
    int nextId = 0;
};

// Get an entity pointer or nullptr from an id
v2Entity* gResolveEntity(int id);

//
//  Template implementation
//

template<typename _t>
_t& v2Entity::Get()
{
    size_t tid = GetComponentId<_t>();

    AttemptBind();

    if (!bounded || bounded->values.count(tid) == 0)
        throw nullptr;

    //assert(bounded 
    //    && bounded->values.count(tid) != 0 
    //    && "Need to first bind a value to get it");
        
    return *(_t*)bounded->values.at(tid);
}

template<typename _t>
_t* v2Entity::TryGet()
{
    size_t tid = GetComponentId<_t>();

    AttemptBind();

    if (!bounded || bounded->values.count(tid) == 0)
        return nullptr;

    return (_t*)bounded->values.at(tid);
}

template<typename _t>
bool v2Entity::Has()
{
    return TryGet<_t>() != nullptr;
}

template<typename... _t>
void v2Entity::_bind(_t&... ptr)
{
    // opt-in through lazy init
    if (!bounded)
        bounded = new EntityBinder();

    (bounded->values.emplace(GetComponentId<_t>(), &ptr), ...);
}

template<typename _t>
size_t v2Entity::GetComponentId()
{
    return typeid(_t).hash_code();
    //static int id = s_nextComponentId++;
    //return id;
}