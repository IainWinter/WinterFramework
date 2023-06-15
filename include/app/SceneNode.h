#pragma once

#include "Entity.h"
#include "Physics.h"
#include "Event.h"

class Application;
struct SceneUpdateGroupNode;

struct SceneNode
{
	Application* app;

	EntityWorld entities;
	PhysicsWorld physics;
	EventBus bus;
	EventQueue event;

	std::vector<SceneUpdateGroupNode*> groups;
	std::unordered_set<SceneUpdateGroupNode*> groupsDetached;
	float timeAcc;
	bool inDebugMode;

	bool physicsRunning;

	SceneNode(Application* app);
	~SceneNode();

	void Tick(float deltaTime, float fixedTime);
	void TickUI();

	SceneUpdateGroupNode* NewGroup();
	void DeleteGroup(SceneUpdateGroupNode* group);
	void AttachGroup(SceneUpdateGroupNode* group);
	void DetachGroup(SceneUpdateGroupNode* group);
};