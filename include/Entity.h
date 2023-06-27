#pragma once

#include "util/typedef.h"
#include "Log.h"

#include "entt/entity/registry.hpp"

#include "ext/serial/serial.h"
#include "util/nonce.h"

#include <unordered_set>
#include <unordered_map>
#include <mutex>

// bug: entt tags (empty structs) don't return in list so query t_... breaks, should always have data in component, or fix this!

namespace tuple_helpers
{
	// these cant be generic because with refs we need to use tie, but anything else would want
	// to use make_tuple, so keep these as private instead of global helper functions...
	// https://stackoverflow.com/a/39101723/6772365

	// splitting EntityQuery made these now have to be free, so I'll just put them in a namespace :)

	template <typename T, typename Tuple>
	auto push_front(const T& t, const Tuple& tuple)
	{
		return std::tuple_cat(std::make_tuple(t), tuple);
	}

	template <typename Tuple, std::size_t ... Is>
	auto pop_front_impl(const Tuple& tuple, std::index_sequence<Is...>)
	{
		return std::tie(std::get<1 + Is>(tuple)...);
	}

	template <typename Tuple>
	auto pop_front(const Tuple& tuple)
	{
		return pop_front_impl(tuple, std::make_index_sequence<std::tuple_size<Tuple>::value - 1>());
	}
}

// Question is should functions assert or soft error and just print to log
// if using the engine, then you really don't want a crash because they you need to restart the editor if a script
// fails
// but when working with the framework, its nice to ensure functionality

// asserts turn off in release anyway so a soft error is prob better

template<typename _t>
_t& _GetDefault()
{
	static _t def;
	log_entity("w~Tried to get a component that doesn't exist.");
	return def;
}

struct Entity;
struct EntityWorld;

struct EntityEventHandler
{
	EntityWorld* world;
	std::function<void(Entity)> handler;
	EntityEventHandler(EntityWorld* world, const std::function<void(Entity)>& handler);
	void handle(entt::registry& reg, entt::entity e);
};

// a wrapper so user doesn't need to mess with pointers
struct EntityEvent
{
	EntityEventHandler* instance;
};

template<typename... _t>
struct EntityQuery
{
public:
	using entt_view = typename entt::view<entt::type_list<_t...>>::iterable;
	using entt_itr  = typename entt_view::iterator;
private:
	entt_view m_view;

public:
	struct Iterator
	{
	private:
		entt_itr m_itr;
	public:
		Iterator(const entt_itr& itr) : m_itr(itr) {}
		Iterator& operator++() { m_itr++; return *this; }
		bool operator==(const Iterator& other) const { return m_itr == other.m_itr; }
		bool operator!=(const Iterator& other) const { return m_itr != other.m_itr; }
		std::tuple<_t&...> operator*() { return tuple_helpers::pop_front(*m_itr); }
	};

	EntityQuery(const entt_view& view) : m_view(view) {}
	Iterator begin() { return Iterator(m_view.begin()); }
	Iterator end() { return Iterator(m_view.end()); }
};

// find a better way to do this

template<typename... _t>
struct EntityQueryWithEntity
{
public:
	using entt_view = typename entt::view<entt::type_list<_t...>>::iterable;
	using entt_itr = typename entt_view::iterator;
private:
	EntityWorld* m_owning;
	entt_view m_view;

public:
	struct Iterator
	{
	private:
		EntityWorld* m_owning; // only for Entity return
		entt_itr m_itr;
	public:
		Iterator(const entt_itr& itr, EntityWorld* owning) : m_itr(itr), m_owning(owning) {}
		Iterator& operator++() { m_itr++; return *this; }
		bool operator==(const Iterator& other) const { return m_itr == other.m_itr; }
		bool operator!=(const Iterator& other) const { return m_itr != other.m_itr; }
		std::tuple<Entity, _t&...> operator*()
		{ 
			return tuple_helpers::push_front(
				Entity(std::get<0>(*m_itr), m_owning),
				tuple_helpers::pop_front(*m_itr)
			);
		}
	};

	EntityQueryWithEntity(const entt_view& view, EntityWorld* owning) : m_view(view), m_owning(owning) {}
	Iterator begin() { return Iterator(m_view.begin(), m_owning); }
	Iterator end() { return Iterator(m_view.end(), m_owning); }
};

struct EntityWorld
{
private:
	entt::registry m_registry;
	
	// deferring deletes to the end of frames to allow events to be processed
	std::unordered_set<entt::entity> m_deferDelete;
	std::mutex m_deferDeleteMutex;

	// keep track of event handlers
	std::unordered_set<EntityEventHandler*> m_handlers;

	friend struct Entity;

public:
	template<typename... _t>
	EntityQuery<_t...> Query()
	{
		return EntityQuery<_t...>(m_registry.view<_t...>().each());
	}

	template<typename... _t>
	EntityQueryWithEntity<_t...> QueryWithEntity()
	{
		return EntityQueryWithEntity<_t...>(m_registry.view<_t...>().each(), this);
	}

	template<typename _t> 
	_t& First()
	{
		EntityQuery<_t> query = Query<_t>();

		if (query.begin() == query.end())
		{
			return _GetDefault<_t>();
		}

		return std::get<0>(*query.begin());
	}

	template<typename... _t> 
	Entity FirstEntity();

	template<typename... _t> 
	int GetNumberOf()
	{
		int count = 0;
		EntityQuery<_t...> query = Query<_t...>();
		for (auto itr = query.begin(); itr != query.end(); ++itr) count += 1;
		return count;
	}

	Entity Create();
	Entity Wrap(u32 id);
	void ExecuteDeferredDeletions();
	void Clear();

	int NumberOfEntities() const;

	std::vector<Entity> GetAllEntities();

	entt::registry& entt() { return m_registry; }

	// event handlers

	template<typename _c>
	EntityEvent OnAdd(const std::function<void(Entity)>& func)
	{
		EntityEventHandler* e = new EntityEventHandler(this, func);
		m_registry.on_construct<_c>().template connect<&EntityEventHandler::handle>(e);
		m_handlers.insert(e);
		return EntityEvent{ e };
	}

	template<typename _c>
	EntityEvent OnRemove(const std::function<void(Entity)>& func)
	{
		EntityEventHandler* e = new EntityEventHandler(this, func);
		m_registry.on_destroy<_c>().template connect<&EntityEventHandler::handle>(e);
		m_handlers.insert(e);
		return EntityEvent{ e };
	}

	template<typename _c>
	void DisconnectOnAdd(const EntityEvent& e)
	{
		m_registry.on_construct<_c>().template disconnect<&EntityEventHandler::handle>(e.instance);
		m_handlers.erase(e.instance);
		delete e.instance;
	}

	template<typename _c>
	void DisconnectOnRemove(const EntityEvent& e)
	{
		m_registry.on_destroy<_c>().template disconnect<&EntityEventHandler::handle>(e.instance);
		m_handlers.erase(e.instance);
		delete e.instance;
	}

private:

// hidden entity functions, called from Entity

	void DeleteEntityNow (entt::entity id);
	void AddDeferedDelete(entt::entity id);
};

struct Entity
{
private:
	EntityWorld* m_owning;
	entt::entity m_handle;

public:
	Entity();
	Entity(entt::entity handle, EntityWorld* owning);
	~Entity();

	Entity(Entity&& move) noexcept;
	Entity(const Entity& copy);
	Entity& operator=(Entity&& move) noexcept;
	Entity& operator=(const Entity& copy);

	bool operator==(const Entity& other) const;
	bool operator!=(const Entity& other) const;

	// shoddy, match this with transform

	Entity& SetParent(const Entity& entity);
	Entity GetParent();

	std::vector<Entity>& GetChildren();

	void DestroyChildren();

	Entity& SetName(const char* name);
	const std::string& GetName();

	Entity Clone();
	
	u32 Id() const;
	u32 raw_id() const; // doesn't check for if this id is valid or not
	EntityWorld* Owning() const;
	
	bool IsAlive() const;
	void Destroy();

	bool IsAliveAtEndOfFrame() const;
	void DestroyAtEndOfFrame() const; // thread safe and guards against double defer deletes

	Entity& OnDestroy(const std::function<void(Entity)>& func);

	// components from a runtime id

	bool Has(meta::id_type component) const;
	meta::any Get(meta::id_type component) const;
	void Add(meta::id_type component);
	void Remove(meta::id_type component);

	void Add(const meta::any& any);
	std::vector<meta::any> GetComponents();

	// components from a templated type

	template<typename... _t>
	bool Has() const
	{
		assert_is_valid();
		return m_owning->m_registry.all_of<_t...>(m_handle);
	}

	template<typename... _t>
	bool HasAny() const
	{
		assert_is_valid();
		return m_owning->m_registry.any_of<_t...>(m_handle);
	}

	template<typename _t>
	_t& Get() const
	{
		assert_is_valid();
		assert_has_components<_t>();
		return std::get<0>(GetAll<_t>());
	}

	template<typename _t>
	_t* TryGet() const
	{
		assert_is_valid();

		if (Has<_t>())
			return &Get<_t>();

		return nullptr;
	}

	template<typename... _t>
	std::tuple<_t&...> GetAll() const
	{
		assert_is_valid();
		assert_has_components<_t...>();
		return m_owning->m_registry.get<_t...>(m_handle);
	}

	template<typename _t, typename... _args>
	_t& Add(_args&&... args)
	{
		assert_is_valid();
		assert_no_components<_t>();
		return m_owning->m_registry.emplace<_t>(m_handle, std::forward<_args>(args)...);
	}

	template<typename... _t>
	Entity& AddAll(const _t&... components)
	{
		(Add<_t>(_t(components)),...);
		return *this;
	}

	template<typename _t>
	void Remove()
	{
		assert_has_components<_t>();
		m_owning->m_registry.remove<_t>(m_handle);
	}

	// Asserts
	                         void assert_is_valid()       const { assert(IsAlive()        && "Entity is not valid"); }
	template<typename... _t> void assert_no_components()  const { assert(!HasAny<_t...>() && "Entity already contains one of these components"); }
	template<typename... _t> void assert_has_components() const { assert(Has<_t...>()     && "Entity doesn't contain one of these components"); }
};

template<typename... _t>
struct EntityWith : Entity
{
	EntityWith()
	{
		// no assurance on empty...
	}

	EntityWith(Entity&& move) noexcept
		: Entity(std::move(move))
	{
		assert_has_components<_t...>();
	}
	EntityWith& operator=(Entity&& move) noexcept
	{
		*((Entity*)this) = std::move(move);
		assert_has_components<_t...>();
		return *this;
	}
	EntityWith(const Entity& copy)
		: Entity(copy)
	{
		assert_has_components<_t...>();
	}
	EntityWith& operator=(const Entity& copy)
	{
		*((Entity*)this) = copy;
		assert_has_components<_t...>();
		return *this;
	}
};

namespace std {
	template<> 
	struct hash<Entity> { 
		size_t operator()(const Entity& x) const { 
			return x.raw_id(); 
		} 
	};
}

struct EntityMeta
{
	Entity parent;
	std::vector<Entity> children;

	std::string name = "Unnamed Entity";
	std::string id = nonce(16);  // should use a uuid
};

// common components

struct OnDestroyComponent
{
	std::vector<std::function<void(Entity)>> funcs;

	void Fire(Entity e)
	{
		for (auto& func : funcs) func(e);
	}
};

//
//	template impl
//

// only FirstEntity is here to define Entity, but all functions should be impled here, not in class

template<typename ..._t>
inline Entity EntityWorld::FirstEntity()
{
	EntityQueryWithEntity<_t...> query = QueryWithEntity<_t...>();
	if (query.begin() == query.end()) return Entity();
	return std::get<0>(*query.begin());
}
