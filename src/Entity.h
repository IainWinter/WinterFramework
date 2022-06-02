#pragma once

#include "Defines.h"
#include "entt/entity/registry.hpp"

struct Entity;
struct EntityWorld;
struct System;
using Order = void*;

namespace tuple_helpers
{
	// these cant be generic because with refs we need to use tie, but anything else would want
	// to use make_tuple, so keep these as private instead of global helper functions...
	// https://stackoverflow.com/a/39101723/6772365

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

	EntityQuery(const entt_view& view, EntityWorld* owning) : m_view(view) {}
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
	std::vector<System*> m_systems; // updater functions
	
	friend struct Entity;
	friend struct System;

public:
	Entity Create();

	const std::vector<System*>& GetSystems() const
	{
		return m_systems;
	}

	// adding systems
	// system constructors should be only default init of values
	// wait until Start to do any work

	template<typename _t>
	Order AddSystem(const _t& system_toCopy)
	{
		System* system = new _t(system_toCopy);
		system->m_owning = this;
		return (Order)m_systems.emplace_back(system);
	}

	template<typename _t>
	Order AddSystemAfter(Order after, const _t& system_toCopy)
	{
		auto itr = m_systems.begin();
		for (; itr != m_systems.end(); ++itr) if (*itr == after) break;
		
		System* system = new _t(system_toCopy);
		system->m_owning = this;
		return (Order)*m_systems.emplace(itr, system);
	}

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

	u32 Id() const
	{
		assert_is_valid();
		return (u32)m_handle; // isnt there like a smuggle functions for this?
	}

	void Destroy()
	{
		assert_is_valid();
		m_owning->m_registry.destroy(m_handle);
		m_owning = nullptr;
		m_handle = {};
	}

	bool IsAlive() const
	{
		return !!m_owning;
	}

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

	template<typename... _t>
	std::tuple<_t&...> GetAll()
	{
		assert_is_valid();
		assert_has_components<_t...>();
		return m_owning->m_registry.get<_t...>(m_handle);
	}

	template<typename _t>
	_t& Get()
	{
		assert_is_valid();
		assert_has_components<_t>();
		return std::get<0>(GetAll<_t>());
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

	// Asserts
	                         void assert_is_valid()       const { assert(IsAlive()        && "Entity is not valid"); }
	template<typename... _t> void assert_no_components()  const { assert(!HasAny<_t...>() && "Entity already contains one of these components"); }
	template<typename... _t> void assert_has_components() const { assert(Has<_t...>()     && "Entity doesnt contains one of these components"); }
};

struct System
{
private:
	EntityWorld* m_owning;
	friend struct EntityWorld;

protected:
	template<typename... _t> EntityQuery<_t...>           Query()           { return m_owning->Query<_t...>(); }
	template<typename... _t> EntityQueryWithEntity<_t...> QueryWithEntity() { return m_owning->QueryWithEntity<_t...>(); }

public:
	virtual void Update() {}
	virtual void FixedUpdate() {}
	virtual void UI() {}
};

// impl, could go into cpp but these funcs are tiny

inline Entity EntityWorld::Create()
{
	return Entity(m_registry.create(), this);
}

// remove this, or put in an engine file with a bunch of globals...
inline static EntityWorld& GetWorld()
{
	static EntityWorld world;
	return world;
}