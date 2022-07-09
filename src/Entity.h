#pragma once

#include "Defines.h"
#include "entt/entity/registry.hpp"
#include <unordered_set>
#include <unordered_map>
#include <mutex>

// entt tags (empty structs) dont return in list so query t_... breaks, should always have data in component, or fix this!

struct Entity;
struct EntityWorld;
using Order = void*;

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
	struct EntityState
	{
		std::vector<std::function<void(Entity)>> onDestroy;
	};

	entt::registry m_registry;
	std::unordered_map<entt::entity, EntityState> m_events;
	std::unordered_set<entt::entity> m_deferDelete;
	std::mutex m_deferDeleteMutex;

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

	void ExecuteDeferdDeletions()
	{
		for (const entt::entity& handle : m_deferDelete)
		{
			DeleteEntityNow(handle);
		}
		m_deferDelete.clear();
	}
	
	void Clear()
	{
		m_registry.clear();
	}

	Entity Create();
	Entity Wrap(u32 id);

// hidden entity functions, called from Entity

private:
	void DeleteEntityNow(entt::entity id);

	void AddDeferedDelete(entt::entity id)
	{
		std::unique_lock lock(m_deferDeleteMutex);
		m_deferDelete.insert(id);
	}

// events

private:

	void         RemoveState(entt::entity id) {        m_events.erase(id); }
	bool         HasState   (entt::entity id) { return m_events.find(id) != m_events.end(); }
	EntityState& AssureState(entt::entity id) { return m_events[id]; }
	EntityState& GetState   (entt::entity id) { return m_events.at(id); }
};

struct Entity
{
private:
	entt::entity m_handle;
	EntityWorld* m_owning;

public:
	Entity()
		: m_handle (entt::null)
		, m_owning (nullptr)
	{}

	Entity(entt::entity handle, EntityWorld* owning)
		: m_handle (handle)
		, m_owning (owning)
	{}

	~Entity() // not needed but will causes proper assert to be called if used after delete
	{
		m_owning = nullptr;
		m_handle = entt::null;
	}

	bool operator==(const Entity& other) const { return Id() == other.Id(); }
	bool operator!=(const Entity& other) const { return Id() != other.Id(); }

	u32 Id() const
	{
		assert_is_valid();
		return raw_id();
	}

	// doesnt check for if this id is valid or not
	u32 raw_id() const
	{
		return (u32)m_handle; // isnt there like a smuggle functions for this?
	}

	bool IsAliveAtEndOfFrame() const
	{
		return IsAlive()
			&& m_owning->m_deferDelete.find(m_handle) == m_owning->m_deferDelete.end();
	}

	bool IsAlive() const
	{
		return !!m_owning && m_owning->m_registry.valid(m_handle);
	}

	void Destroy() const
	{
		assert_is_valid();
		m_owning->DeleteEntityNow(m_handle);
	}

	void Destroy()
	{
		const Entity* me = this;
		me->Destroy();

		m_owning = nullptr;
		m_handle = entt::null;
	}

	// threadsafe and guards against double defer deletes
	void DestroyAtEndOfFrame() const
	{
		m_owning->AddDeferedDelete(m_handle);
	}

	// could use template meta nonsense to remove Get/GetAll
	// this api isnt the best :(s

	// Testing components

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

	// Getting components

	template<typename _t>
	_t& Get()
	{
		return const_cast<_t&>(std::as_const(*this).Get<_t>());
	}

	template<typename... _t>
	std::tuple<_t&...> GetAll()
	{
		//return const_cast<std::tuple<_t&...>>(std::as_const(*this).GetAll<_t...>());
		// annoying

		assert_is_valid();
		assert_has_components<_t...>();
		return m_owning->m_registry.get<_t...>(m_handle);
	}

	template<typename _t>
	const _t& Get() const
	{
		assert_is_valid();
		assert_has_components<_t>();
		return std::get<0>(GetAll<_t>());
	}

	template<typename... _t>
	std::tuple<const _t&...> GetAll() const
	{
		assert_is_valid();
		assert_has_components<_t...>();
		return m_owning->m_registry.get<_t...>(m_handle);
	}

	// Adding components

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

	// Removing compoennts

	template<typename... _t>
	void Remove()
	{
		assert_has_components<_t...>();
		m_owning->m_registry.remove<_t...>(m_handle);
	}

	// Listeners

	Entity& OnDestroy(const std::function<void(Entity)>& func)
	{
		m_owning->AssureState(m_handle).onDestroy.push_back(func);
		return *this;
	}

	// Cloning

	Entity Clone()
	{
		Entity entity;

		for (auto [id, storage] : m_owning->m_registry.storage())  // like visit function
		{
			if (storage.contains(m_handle))
			{
				storage.emplace(entity.m_handle, storage.get(m_handle));
			}
		}

		return entity;
	}

	// Asserts
	                         void assert_is_valid()       const { assert(IsAlive()        && "Entity is not valid"); }
	template<typename... _t> void assert_no_components()  const { assert(!HasAny<_t...>() && "Entity already contains one of these components"); }
	template<typename... _t> void assert_has_components() const { assert(Has<_t...>()     && "Entity doesnt contain one of these components"); }

	// Copy and Move

	Entity(Entity&& move) noexcept
		: m_handle (move.m_handle)
		, m_owning (move.m_owning)
	{
		move.m_handle = entt::null;
		move.m_owning = nullptr;
	}
	Entity& operator=(Entity&& move) noexcept
	{
		m_handle = move.m_handle;
		m_owning = move.m_owning;
		move.m_handle = entt::null;
		move.m_owning = nullptr;
		return *this;
	}
	Entity(const Entity& move)
		: m_handle(move.m_handle)
		, m_owning(move.m_owning)
	{}
	Entity& operator=(const Entity& move)
	{
		m_handle = move.m_handle;
		m_owning = move.m_owning;
		return *this;
	}
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
	template<> struct hash<Entity> { size_t operator()(const Entity& x) const { return x.Id(); } };
}

// impl here for Entity def

inline Entity EntityWorld::Create()
{
	return Wrap((u32)m_registry.create());
}

inline Entity EntityWorld::Wrap(u32 id)
{
	return Entity((entt::entity)id, this);
}

inline void EntityWorld::DeleteEntityNow(entt::entity id)
{
	if (HasState(id))
	{
		Entity e = Wrap((u32)id);
		for (auto& func : GetState(id).onDestroy) func(e);
		RemoveState(id);
	}

	m_registry.destroy(id);
}