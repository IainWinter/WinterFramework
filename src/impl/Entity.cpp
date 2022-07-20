#pragma once

#include "Entity.h"

Entity EntityWorld::Create()
{
	return Wrap((u32)m_registry.create());
}

Entity EntityWorld::Wrap(u32 id)
{
	return Entity((entt::entity)id, this);
}

void EntityWorld::ExecuteDeferdDeletions()
{
	for (const entt::entity& handle : m_deferDelete)
	{
		DeleteEntityNow(handle);
	}
	m_deferDelete.clear();
}
	
void EntityWorld::Clear()
{
	m_registry.clear();
}

void EntityWorld::AddDeferedDelete(entt::entity id)
{
	std::unique_lock lock(m_deferDeleteMutex);
	m_deferDelete.insert(id);
}

void EntityWorld::DeleteEntityNow(entt::entity id)
{
	if (HasState(id))
	{
		Entity e = Wrap((u32)id);
		for (auto& func : GetState(id).onDestroy) func(e);
		RemoveState(id);
	}

	m_registry.destroy(id);
}

void                      EntityWorld::RemoveState(entt::entity id) {        m_events.erase(id); }
bool                      EntityWorld::HasState   (entt::entity id) { return m_events.find(id) != m_events.end(); }
EntityWorld::EntityState& EntityWorld::AssureState(entt::entity id) { return m_events[id]; }
EntityWorld::EntityState& EntityWorld::GetState   (entt::entity id) { return m_events.at(id); }

Entity::Entity()
	: m_handle (entt::null)
	, m_owning (nullptr)
{}

Entity::Entity(entt::entity handle, EntityWorld* owning)
	: m_handle (handle)
	, m_owning (owning)
{}

Entity::~Entity() // not needed but will causes proper assert to be called if used after delete
{
	m_owning = nullptr;
	m_handle = entt::null;
}

Entity::Entity(Entity&& move) noexcept
	: m_handle(move.m_handle)
	, m_owning(move.m_owning)
{
	move.m_handle = entt::null;
	move.m_owning = nullptr;
}
Entity& Entity::operator=(Entity&& move) noexcept
{
	m_handle = move.m_handle;
	m_owning = move.m_owning;
	move.m_handle = entt::null;
	move.m_owning = nullptr;
	return *this;
}
Entity& Entity::operator=(const Entity& move)
{
	m_handle = move.m_handle;
	m_owning = move.m_owning;
	return *this;
}
Entity::Entity(const Entity& move)
	: m_handle(move.m_handle)
	, m_owning(move.m_owning)
{}

bool Entity::operator==(const Entity& other) const { return Id() == other.Id(); }
bool Entity::operator!=(const Entity& other) const { return Id() != other.Id(); }

Entity Entity::Clone()
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

u32 Entity::Id() const
{
	assert_is_valid();
	return raw_id();
}

u32 Entity::raw_id() const
{
	return (u32)m_handle; // isnt there like a smuggle functions for this?
}

EntityWorld* Entity::Owning() const
{
	assert_is_valid();
	return m_owning;
}

bool Entity::IsAlive() const
{
	return !!m_owning && m_owning->m_registry.valid(m_handle);
}

void Entity::Destroy() const
{
	assert_is_valid();
	m_owning->DeleteEntityNow(m_handle);
}

void Entity::Destroy()
{
	const Entity* me = this;
	me->Destroy();

	m_owning = nullptr;
	m_handle = entt::null;
}

bool Entity::IsAliveAtEndOfFrame() const
{
	return IsAlive()
		&& m_owning->m_deferDelete.find(m_handle) == m_owning->m_deferDelete.end();
}

void Entity::DestroyAtEndOfFrame() const
{
	m_owning->AddDeferedDelete(m_handle);
}

Entity& Entity::OnDestroy(const std::function<void(Entity)>& func)
{
	m_owning->AssureState(m_handle).onDestroy.push_back(func);
	return *this;
}