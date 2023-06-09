#include "app/Scene.h"
#include "app/SceneNode.h"

Scene::Scene()
	: m_node (nullptr)
{}

Scene::Scene(SceneNode* node)
	: m_node (node)
{}

SceneUpdateGroup Scene::CreateGroup()
{
	return SceneUpdateGroup(m_node->NewGroup(), m_node);
}

void Scene::DestroyGroup(SceneUpdateGroup* group)
{
	m_node->DeleteGroup(group->m_node);
	group->m_node = nullptr;
}

void Scene::AttachGroup(SceneUpdateGroup& group)
{
	m_node->AttachGroup(group.m_node);
}

void Scene::DetachGroup(SceneUpdateGroup& group)
{
	m_node->DetachGroup(group.m_node);
}

void Scene::SetPhysicsRunning(bool running)
{
	m_node->physicsRunning = running;
}
