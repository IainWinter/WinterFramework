#include "ext/EntityPrefab.h"
#include "ext/serial/serial_bin.h"
#include <fstream>

EntityPrefab::EntityPrefab(const std::string& filename)
{
	std::ifstream in(filename);
	if (in.is_open())
	{
		bin_reader(in).read(*this);
	}
}

EntityPrefab::EntityPrefab(Entity entity)
{
	
}

void EntityPrefab::Add(const meta::any& component)
{
	if (!component.type()->has_prop("is_component"))
	{
		log_entity("Tried to add non-component to prefab. Type: %s", component.type()->name());
		return;
	}

	// todo: see if we need to explicitly make a copy or does copy constructor suffice
	//meta::any copy =  component.type()->construct().copy_in(component.data());

	for (meta::any& any : m_components) // replace if already in prefab
	{
		if (any.type_id() == component.type_id())
		{
			any = component;
			return;
		}
	}

	// or push

	m_components.push_back(component);

	//// take a copy of the component
	//// todo: see if we need to explicitly make a copy or does copy constructor suffice

	//m_components[component.type_id()] = component;// .type()->construct().copy_in(component.data());
}

void EntityPrefab::Remove(const meta::id_type& type)
{
	for (auto itr = m_components.begin(); itr != m_components.end(); ++itr)
	{
		if (itr->type_id() == type)
		{
			m_components.erase(itr);
			return;
		}
	}

	log_entity("Tried to remove a component not in the prefab. Type: %zd", type);
}

const std::vector<meta::any>& EntityPrefab::GetComponents() const
{
	return m_components;
}