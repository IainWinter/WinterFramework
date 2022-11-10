#pragma once

#include "Defines.h"
#include "Log.h"
#include "entt/entity/registry.hpp"
#include "entt/meta/meta.hpp"
#include "entt/meta/resolve.hpp"

#include "ext/serial/serial.h" // serial should be a util

#include <unordered_set>
#include <unordered_map>
#include <mutex>

// bug: entt tags (empty structs) dont return in list so query t_... breaks, should always have data in component, or fix this!

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

struct Entity;
struct EntityWorld;

struct EntityEventHandler
{
	EntityWorld* world;
	std::function<void(Entity)> handler;
	EntityEventHandler(EntityWorld* world, const std::function<void(Entity)>& handler);
	void handle(entt::registry& reg, entt::entity e);
};

// a wrapper so user doesnt need to mess with pointers
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
	
	// defering deletes to the end of frames to allow events to be processed
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
			log_world("e~First failed, no entity exists with component");
			throw nullptr;
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
	void ExecuteDeferdDeletions();
	void Clear();

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

	Entity& SetParent(const Entity& entity);
	Entity GetParent();

	Entity& SetName(const char* name);
	const char* GetName();

	Entity Clone();
	
	u32 Id() const;
	u32 raw_id() const; // doesnt check for if this id is valid or not
	EntityWorld* Owning() const;
	
	bool IsAlive() const;
	void Destroy() const;
	void Destroy();

	bool IsAliveAtEndOfFrame() const;
	void DestroyAtEndOfFrame() const; // threadsafe and guards against double defer deletes

	Entity& OnDestroy(const std::function<void(Entity)>& func);

	// could use template meta nonsense to remove Get/GetAll
	// this api isnt the best :(

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

	template<typename... _t>
	std::tuple<_t&...> GetAll()  const
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

		if constexpr (!std::is_same<_t, EntityMeta>())
		{
			Get<EntityMeta>().components.emplace(meta::name<_t>());
		}

		return m_owning->m_registry.emplace<_t>(m_handle, std::forward<_args>(args)...);
	}

	//template<typename... _t>
	//Entity& AddAll(const _t&... components)
	//{
	//	(Add<_t>(_t(components)),...);
	//	return *this;
	//}

	template<typename _t>
	void Remove()
	{
		assert_has_components<_t>();

		if constexpr (!std::is_same<_t, EntityMeta>()) // should assert that you arn't removing this as it is essential
		{
			Get<EntityMeta>().components.erase(meta::name<_t>());
		}

		m_owning->m_registry.remove<_t>(m_handle);
	}

	// Asserts
	                         void assert_is_valid()       const { assert(IsAlive()        && "Entity is not valid"); }
	template<typename... _t> void assert_no_components()  const { assert(!HasAny<_t...>() && "Entity already contains one of these components"); }
	template<typename... _t> void assert_has_components() const { assert(Has<_t...>()     && "Entity doesnt contain one of these components"); }
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

// common components

struct OnDestroyComponent
{
	std::vector<std::function<void(Entity)>> funcs;

	void Fire(Entity e)
	{
		for (auto& func : funcs) func(e);
	}
};

// this is for looping over every entity with EntityQuery
struct EntityMeta
{
	Entity parent;
	const char* name = "Unnamed Entity";
	std::unordered_set</*const char**/std::string> components;
};

// template impl

// only FirstEntity is here to define Entity, but all functions should be impled here, not in class

template<typename ..._t>
inline Entity EntityWorld::FirstEntity()
{
	EntityQueryWithEntity<_t...> query = QueryWithEntity<_t...>();
	if (query.begin() == query.end()) return Entity();
	return std::get<0>(*query.begin());
}
