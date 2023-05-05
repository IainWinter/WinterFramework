#include "app/UpdateTree.h"
#include "app/Update.h"

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

UpdateGroup& SceneUpdate::CreateGroup(const char* name)
{
	UpdateGroup* group = new UpdateGroup();
	m_groups.push_back(group);
	
	return *group;
}

std::vector<SystemBase*> SceneUpdate::GetUpdateOrder()
{
	std::vector<SystemBase*> list;
	
	for (UpdateGroup* group : m_groups)
	for (SystemBase* update : group->m_updates)
		list.push_back(update);

	return list;
}
