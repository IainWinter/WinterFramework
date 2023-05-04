#include "app/Scene.h"

Scene::Scene(SceneNode* node)
	: m_node (node)
{}

UpdateTreeNode& Scene::CreateGroup(const char* name)
{
	return m_node->update.CreateGroup(name);
}