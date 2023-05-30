#include "app/UpdateTree.h"
#include "app/Update.h"

UpdateGroup::UpdateGroup(const std::string& name)
	: m_name (name)
{}

UpdateGroup::~UpdateGroup()
{
	for (SystemBase* update : m_updates)
		delete update;
}

void UpdateGroup::AddUpdate(SystemBase* update)
{
	m_updates.push_back(update);
}

SceneUpdate::~SceneUpdate()
{
	for (UpdateGroup* group : m_groups)
		delete group;
}

UpdateGroup& SceneUpdate::CreateGroup(const std::string& name)
{
	UpdateGroup* group = new UpdateGroup(name);
	m_groups.push_back(group);
	
	return *group;
}

void SceneUpdate::DestroyGroup(const std::string& name)
{
	auto itr = std::find_if(m_groups.begin(), m_groups.end(), 
		[this, &name](UpdateGroup* group) { return group->m_name == name; }
	);

	if (itr == m_groups.end())
		return;

	delete *itr;
	m_groups.erase(itr);
}

std::vector<SystemBase*> SceneUpdate::GetUpdateOrder()
{
	std::vector<SystemBase*> list;
	
	for (UpdateGroup* group : m_groups)
	for (SystemBase* update : group->m_updates)
		list.push_back(update);

	return list;
}
