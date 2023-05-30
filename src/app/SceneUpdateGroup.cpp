#include "app/SceneUpdateGroup.h"
#include "app/SceneUpdateGroupNode.h"

SceneUpdateGroup::SceneUpdateGroup()
	: m_node (nullptr)
{}

SceneUpdateGroup::SceneUpdateGroup(SceneUpdateGroupNode* node)
	: m_node (node)
{}

void SceneUpdateGroup::TakeOwnershipOfSystem(SystemBase* system)
{
	m_node->TakeOwnershipOfSystem(system);
}
