#include "app/SceneUpdateGroup.h"
#include "app/SceneUpdateGroupNode.h"

SceneUpdateGroup::SceneUpdateGroup()
	: m_node (nullptr)
{}

SceneUpdateGroup::SceneUpdateGroup(SceneUpdateGroupNode* node, SceneNode* sceneNode)
	: m_node      (node)
	, m_sceneNode (sceneNode)
{}

SceneUpdateGroup& SceneUpdateGroup::InitNow()
{
	m_node->Init(m_sceneNode);
	m_node->Attach();

	return *this;
}

void SceneUpdateGroup::TakeOwnershipOfSystem(SystemBase* system, const char* name)
{
	m_node->TakeOwnershipOfSystem(system, name);
}
