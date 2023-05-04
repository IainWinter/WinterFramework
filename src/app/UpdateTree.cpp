#include "app/UpdateTree.h"
#include "app/Update.h"

UpdateTreeNode::~UpdateTreeNode()
{
	delete m_update;

	for (UpdateTreeNode* node : m_children)
		delete node;
}

UpdateTreeNode& UpdateTreeNode::Then(SystemBase* update)
{
	if (!m_update)
	{
		m_update = update;
	}

	else
	{
		UpdateTreeNode* child = new UpdateTreeNode();
		child->m_update = update;
		m_children.push_back(child);
	}

	return *this;
}

UpdateTreeNode& UpdateTree::CreateGroup(const char* name)
{
	UpdateTreeNode* group = new UpdateTreeNode();
	m_roots.push_back(group);
	return *group;
}

std::vector<SystemBase*> UpdateTree::GetOrderedList()
{
	std::vector<SystemBase*> list;
	Walk([&list](SystemBase* s) { list.push_back(s); });
	return list;
}
