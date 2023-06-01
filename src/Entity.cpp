#include "Entity.h"

EntityEventHandler::EntityEventHandler(
	EntityWorld* world, 
	const std::function<void(Entity)>& handler
)
	: world   (world)
	, handler (handler)
{}

void EntityEventHandler::handle(entt::registry& reg, entt::entity e)
{
	handler(world->Wrap((u32)e));
}

Entity EntityWorld::Create()
{
	Entity e = Wrap((u32)m_registry.create());
	e.Add<EntityMeta>();
	return e;
}

Entity EntityWorld::Wrap(u32 id)
{
	return Entity((entt::entity)id, this);
}

void EntityWorld::ExecuteDeferredDeletions()
{
	for (const entt::entity& handle : m_deferDelete)
	{
		Entity e = Wrap((u32)handle);

		if (e.IsAlive())
			e.Destroy();
	}

	m_deferDelete.clear();
}
	
void EntityWorld::Clear()
{
	m_registry.clear();
}

int EntityWorld::NumberOfEntities() const
{
	return (int)m_registry.size();
}

std::vector<Entity> EntityWorld::GetAllEntities()
{
	std::vector<Entity> entities;

	m_registry.each([&](entt::entity e)
	{
		entities.push_back(Wrap((u32)e));
	});

	return entities;
};

void EntityWorld::DeleteEntityNow(entt::entity id)
{
	m_registry.destroy(id);
}

void EntityWorld::AddDeferedDelete(entt::entity id)
{
	std::unique_lock lock(m_deferDeleteMutex);
	m_deferDelete.insert(id);
}

Entity::Entity()
	: m_owning (nullptr)
	, m_handle (entt::null)
{}

Entity::Entity(entt::entity handle, EntityWorld* owning)
	: m_owning (owning)
	, m_handle (handle)
{}

Entity::~Entity() // not needed but will causes proper assert to be called if used after delete
{
	m_owning = nullptr;
	m_handle = entt::null;
}

Entity::Entity(Entity&& move) noexcept
	: m_owning (move.m_owning)
	, m_handle (move.m_handle)
{
	move.m_owning = nullptr;
	move.m_handle = entt::null;
}

Entity::Entity(const Entity& copy)
	: m_owning (copy.m_owning)
	, m_handle (copy.m_handle)
{}

Entity& Entity::operator=(Entity&& move) noexcept
{
	m_owning = move.m_owning;
	m_handle = move.m_handle;
	move.m_owning = nullptr;
	move.m_handle = entt::null;
	return *this;
}

Entity& Entity::operator=(const Entity& copy)
{
	m_owning = copy.m_owning;
	m_handle = copy.m_handle;
	return *this;
}

bool Entity::operator==(const Entity& other) const { return raw_id() == other.raw_id(); }
bool Entity::operator!=(const Entity& other) const { return raw_id() != other.raw_id(); }

Entity& Entity::SetParent(const Entity& entity)
{
	Get<EntityMeta>().parent = entity;
	return *this;
}

Entity Entity::GetParent()
{
	return Get<EntityMeta>().parent;
}

Entity& Entity::SetName(const char* name)
{
	Get<EntityMeta>().name = name;
	return *this;
}

const std::string& Entity::GetName()
{
	return Get<EntityMeta>().name;
}

Entity Entity::Clone()
{
	Entity entity = m_owning->Create();

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
	return (u32)m_handle; // isn't there like a smuggle functions for this?
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

	// this has to be here because if it is a component event,
	// then some of the components get cleaned up before on_destroy gets fired
	if (Has<OnDestroyComponent>())
	{
		Get<OnDestroyComponent>().Fire(*this);
	}

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
	if (Has<OnDestroyComponent>()) Get<OnDestroyComponent>().funcs.push_back(func);
	else                           Add<OnDestroyComponent>().funcs.push_back(func);

	return *this;
}

bool Entity::Has(meta::id_type component) const
{
	entt::sparse_set* store = m_owning->entt().storage(component);
	return store && store->contains(m_handle);
}

meta::any Entity::Get(meta::id_type component) const
{
	entt::sparse_set* store = m_owning->entt().storage(component);
	meta::any any;

	if (store && store->contains(m_handle))
	{
		any = meta::any(meta::get_registered_type(component), store->get(m_handle));
	}

	return any;
}

void Entity::Add(meta::id_type component)
{
	entt::sparse_set* store = m_owning->entt().storage(component);

	if (store && !store->contains(m_handle))
	{
		store->emplace(m_handle);
	}
}

void Entity::Remove(meta::id_type component)
{
	entt::sparse_set* store = m_owning->entt().storage(component);

	if (store && store->contains(m_handle))
	{
		store->remove(m_handle);
	}
}

void Entity::Add(const meta::any& any)
{
	entt::sparse_set* store = m_owning->entt().storage(any.type_id());

	if (store && !store->contains(m_handle))
	{
		store->emplace(m_handle, any.data());
		//void* ptr = store->get(m_handle);
		//any.copy_to(ptr);
	}
}

std::vector<meta::any> Entity::GetComponents()
{
	std::vector<meta::any> data;

	for (auto&& [component, store] : m_owning->entt().storage())
    {
        if (store.contains(m_handle))
        {
            meta::type* type = meta::from_entt(component);
            void* ptr = store.get(m_handle);

            if (!type)
            {
                log_io("e~Failed to serialize type, no type information. %s", entt::type_id(component).name().data());
                continue;
            }

            data.push_back(meta::any(type, ptr));
        }
    }

	return data;
}
