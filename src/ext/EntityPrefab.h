#pragma once

#include "Entity.h"

struct EntityPrefab
{
private:
	std::vector<meta::any> m_components;

public:
	EntityPrefab() = default;
	EntityPrefab(const std::string& filename);

	void Add(const meta::any& component);
	void Remove(const meta::id_type& type);

	const std::vector<meta::any>& GetComponents() const;
};