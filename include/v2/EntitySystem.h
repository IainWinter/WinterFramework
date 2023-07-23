#pragma once

#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <deque>
#include <functional>

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
    int Map(v2Entity* ptr);
    void Update(int id, v2Entity* ptr);
    v2Entity* Get(int id);
    void Remove(int id);

private:
    std::unordered_map<int, v2Entity*> entities;
    int nextId = 0;
};

// Get an entity pointer or nullptr from an id
v2Entity* gResolveEntity(int id);

// A super simple vector-like storage which forces the move constructor
// to be used. Also indexed on the Id of each entity.
// This kinda sucks, should use a sprase set
template<typename _t>
class v2MoveList {
public:
    v2MoveList() {
        data = (_t*)malloc(capacity * sizeof(_t));
    }

    ~v2MoveList() {
        clear();
    }

    _t* at(int id) {
        int index = find_index(id);
        return &data[index];
    }

    void add(_t& item) {
        if (size >= capacity)
            resize(capacity * 2 + 1);

        new (data + size) _t(std::move(item));
        size += 1;
    }

    bool contains(int id) const {
        return find_index(id) < size;
    }

    void remove(int id) {
        int index = find_index(id);

        if (index >= size)
            return;

        data[index].~_t();

        if (size > 1 && index != size - 1)
            new (data + index) _t(std::move(data[size - 1]));

        size -= 1;

        if (size <= capacity / 4 && capacity > 4)
            resize(capacity / 4);
    }

    void clear() {
        for (int i = 0; i < size; i++)
            data[i].~_t();

        free(data);
        data = nullptr;
        capacity = 0;
        size = 0;
    }

    int count() const {
        return size;
    }

    _t* begin() { return data; }
    _t* end() { return data + size; }

    const _t* begin() const { return data; }
    const _t* end() const { return data + size; }

private:
    void resize(int new_size) {
        capacity = new_size;

        // resize
        _t* new_data = (_t*)malloc(capacity * sizeof(_t));
        for (int i = 0; i < size; i++)
            new (&new_data[i]) _t(std::move(data[i]));

        free(data);
        data = new_data;
    }

    int find_index(int id) const {
        int index = 0;
        for (; index < size; index++)
            if (data[index].Id() == id)
                break;

        return index;
    }

private:
    int capacity = 1;
    int growth = 2;

    int size = 0;
    _t* data = nullptr;
};

class v2BasicEntityList
{
public:
    struct v2BasicEntityListIterator
    {
        v2Entity* pointer;
        int size;

        v2BasicEntityListIterator& operator++() {
            pointer = (v2Entity*) ((char*)pointer + size);
            return *this;
        }

        v2Entity& operator*() {
            return *pointer;
        }

        v2Entity* operator->() {
            return pointer;
        }

        bool operator==(const v2BasicEntityListIterator& other) const {
            return pointer == other.pointer;
        }

        bool operator!=(const v2BasicEntityListIterator& other) const {
            return pointer != other.pointer;
        }
    };

    virtual bool contains(int id) const = 0;
    virtual int count() const = 0;

    virtual void add_default() = 0;
    virtual void remove(int id) = 0;
    virtual void commit() = 0;
    virtual void clear() = 0;

    virtual v2BasicEntityListIterator basic_begin() = 0;
    virtual v2BasicEntityListIterator basic_end() = 0;
    
    bool archetype_contains_subset(const std::unordered_set<size_t>& subset) const {
        for (size_t c : subset)
            if (archetype.count(c) == 0)
                return false;
        return true;
    }

protected:
    std::unordered_set<size_t> archetype;

public:
    const char* typeName;
};

// Viewing entities involves copying all points to their data
// into a list one time per frame
class v2BasicEntityView
{
public:
    template<typename... _c>
    struct v2BasicEntityViewIterator
    {
        typename std::vector<std::tuple<_c*...>>::iterator iterator;

        v2BasicEntityViewIterator& operator++() {
            ++iterator;
            return *this;
        }

        bool operator==(const v2BasicEntityViewIterator& other) const {
            return iterator == other.iterator;
        }

        bool operator!=(const v2BasicEntityViewIterator& other) const {
            return iterator != other.iterator;
        }

        std::tuple<_c&...> operator*() {
            return ptrs_to_refs(*iterator, std::make_index_sequence<sizeof...(_c)>{});
        }

    private:
        template<size_t... I>
        std::tuple<_c&...> ptrs_to_refs(std::tuple<_c*...>& c, std::index_sequence<I...>) {
            return std::tie(*std::get<I>(c)...);
        }
    };

    virtual int count() const = 0;

    virtual void reg(v2Entity& entity) = 0;
    virtual void clear() = 0;

    const std::unordered_set<size_t>& get_components() const {
        return components;
    }

    void reg_list(v2BasicEntityList& list)
    {
        auto end = list.basic_end();
        for (auto itr = list.basic_begin(); itr != end; ++itr)
            reg(*itr);
    }

protected:
    std::unordered_set<size_t> components;
};

// Allow a free stored entity to be viewed and allows the changing
// of the archetype everytime reg/move is called
template<typename _t>
class v2EntitySingleton : public v2BasicEntityList
{
public:
    v2EntitySingleton() {
        entity = nullptr;
        has_move = false;
        has_remove = false;
        typeName = typeid(_t).name();
    }

    void reg(_t* entity) {
        this->entity = entity;
        // don't set archetype or size, only register the memory pointer
    }

    int move(_t& entity) {
        int id = entity.Id();
        move_entity = std::move(entity);
        has_move = true;
        return id;
    }

    int count() const override {
        return size;
    }

    bool contains(int id) const override {
        return entity->Id() == id;
    }

    void add_default() override {
        _t t{};
        move(t);
    }

    void remove() {
        remove(0);
    }

    // id is not considered
    void remove(int id) override {
        has_remove = true;
    }

    void commit() override {
        if (!entity)
            return;
        
        if (has_remove) {
            has_remove = false;

            if (on_remove_func)
                on_remove_func(*entity);

            *entity = {}; // reset to default
            size = 0;
        }

        if (has_move) {
            has_move = false;

            // change the archetype
            archetype = move_entity.GetArchetype();

            *entity = std::move(move_entity);
            size = 1;
        }
    }

    void clear() override {
        if (entity)
            *entity = {};
        
        move_entity = {};
        has_move = false;
        has_remove = false;
        size = 0;
    }

    template<typename _callable>
    void on_remove(_callable&& func) {
        on_remove_func = func;
    }

    _t* begin() { return entity; }
    _t* end() { return entity + size; }

    v2BasicEntityListIterator basic_begin() override { return v2BasicEntityListIterator{ begin(), sizeof(_t)}; }
    v2BasicEntityListIterator basic_end() override { return v2BasicEntityListIterator{ end(), sizeof(_t)}; }

private:
    _t* entity;
    _t move_entity;
    bool has_move;
    bool has_remove;
    int size = 0;

    std::function<void(_t&)> on_remove_func;
};

// Store entities of a given type in a tight array
template<typename _t>
class v2EntityList : public v2BasicEntityList
{
public:
    v2EntityList() {
        archetype = _t().GetArchetype(); // annoying but need to call Bind, which cannot be static
        typeName = typeid(_t).name();
    }

    int move(_t& entity) {
        int id = entity.Id();
        add_list.emplace_back(std::move(entity));
        return id;
    }

    int count() const override {
        return data.count();
    }

    bool contains(int id) const override {
        return data.contains(id);
    }

    void add_default() override {
        _t t{};
        move(t);
    }

    void remove(int id) override {
        remove_list.push_back(id);
    }
    
    void commit() override {
        for (int id : remove_list)
            data.remove(id);

        for (_t& add : add_list)
            data.add(add);

        add_list.clear();
        remove_list.clear();
    }

    void clear() override {
        data.clear();
        add_list.clear();
        remove_list.clear();
    }

    // allow iteration
    _t* begin() { return data.begin(); }
    _t* end() { return data.end(); }

    const _t* begin() const { return data.begin(); }
    const _t* end() const { return data.end(); }

    v2BasicEntityListIterator basic_begin() override { return v2BasicEntityListIterator{ begin(), sizeof(_t) }; }
    v2BasicEntityListIterator basic_end() override { return v2BasicEntityListIterator{ end(), sizeof(_t) }; }

private:
    v2MoveList<_t> data;

    std::deque<_t> add_list; // use deque for no copies
    std::vector<int> remove_list;
};

// Returns a tuple of components for each entity
template<typename... _c>
class v2EntityView : public v2BasicEntityView
{
public:
    using v2EntityViewIterator = v2BasicEntityViewIterator<_c...>;

public:
    v2EntityView() {
        components = { typeid(_c).hash_code()... };
    }

    int count() const override {
        return ptrs.size();
    }

    void reg(v2Entity& entity) override {
        ptrs.push_back({ &entity.Get<_c>() ... });
    }

    void clear() override {
        ptrs.clear();
    }

    v2EntityViewIterator begin() { return { ptrs.begin() }; }
    v2EntityViewIterator end() { return { ptrs.end() }; }

private:
    std::vector<std::tuple<_c*...>> ptrs;
};

// Returns a v2Entity along with its components
template<typename... _c>
class v2EntityView<v2Entity, _c...> : public v2BasicEntityView
{
public:
    using v2EntityViewIterator = v2BasicEntityViewIterator<v2Entity, _c...>;

public:
    v2EntityView() {
        components = { typeid(_c).hash_code()... };
    }

    int count() const override {
        return ptrs.size();
    }

    void reg(v2Entity& entity) override {
        ptrs.push_back({ &entity, &entity.Get<_c>() ... });
    }

    void clear() override {
        ptrs.clear();
    }

    // find if an entity is alive without knowing its type
    // this can be less expensive than searching the entire scene
    // if the view only iterates a small subset of entities
    bool contains(int id) const {
        for (const auto& tpl : ptrs)
            if (std::get<0>(tpl)->Id() == id)
                return true;
        return false;
    }

    v2EntityViewIterator begin() { return { ptrs.begin() }; }
    v2EntityViewIterator end() { return { ptrs.end() }; }

private:
    std::vector<std::tuple<v2Entity*, _c*...>> ptrs;
};

// Inherit from this to store scene data and register lists and views
class v2EntitySceneData
{
public:
    int count() const {
        int size = 0;
        for (v2BasicEntityList* list : lists)
            size += list->count();

        return size;
    }

    // if the entity type is unknown, try to remove from all lists
    // Could store list in id if needed
    void remove(int id) {
        for (v2BasicEntityList* list : lists) {
            if (list->contains(id)) {
                list->remove(id);
                break;
            }
        }
    }

    void commit() {
        for (v2BasicEntityList* list : lists)
            list->commit();

        for (v2BasicEntityView* view : views)
        {
            view->clear();
            for (v2BasicEntityList* list : lists)
                if (list->archetype_contains_subset(view->get_components()))
                    view->reg_list(*list);
        }
    }

    void clear() {
        for (v2BasicEntityList* list : lists)
            list->clear();

        commit();
    }

public:
    std::vector<v2BasicEntityList*> lists;
    std::vector<v2BasicEntityView*> views;
};

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
