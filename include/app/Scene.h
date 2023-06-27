#pragma once

#include "app/SceneUpdateGroup.h"

struct SceneNode;
class Application;

// Scenes hold the entity state. Many can be used at once,
// but can only interact through event passing up to the app bus.
// 
// Scenes also hold the systems for updating their entity state
// which are stored in SceneUpdateGroups. This allows multiple systems
// which always work in tandem to be referred to by a single identifier.
class Scene
{
public:
	Scene();
	Scene(SceneNode* node);

public:
	SceneUpdateGroup CreateGroup();
	void DestroyGroup(SceneUpdateGroup* group);
	void AttachGroup(SceneUpdateGroup& group);
	void DetachGroup(SceneUpdateGroup& group);

	void SetPhysicsRunning(bool running);

	// just for testing a new system that
	// cannot auto reg physics
	// not needed outside of v2/EntitySystem stuff
	SceneNode* _get_node();

private:
	friend class Application;

private:
	SceneNode* m_node;
};