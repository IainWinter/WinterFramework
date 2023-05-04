#pragma once

#include "app/SceneNode.h"
#include "app/UpdateTree.h"

class Scene
{
public:
	Scene(SceneNode* node);

public:
	UpdateTreeNode& CreateGroup(const char* name);

private:
	friend struct Application;

private:
	SceneNode* m_node;
};