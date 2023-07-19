#pragma once

#include "v2/EntitySystem.h"
#include "Physics.h"
#include "Event.h"

class Application;
struct SceneUpdateGroupNode;

struct SceneNode
{
	Application* app;

	//EntityWorld entities;
	//entity_scene_data* entities;
	v2EntitySceneData* data;
	PhysicsWorld physics;
	EventBus bus;
	EventQueue event;

	std::vector<SceneUpdateGroupNode*> groups;
	std::unordered_set<SceneUpdateGroupNode*> groupsDetached;
	float timeAcc;
	bool inDebugMode;
	bool physicsRunning;

	// takes reference to app 
	// takes ownership of data
	SceneNode(Application* app, v2EntitySceneData* data);
	~SceneNode();

	void Tick(float deltaTime, float fixedTime);
	void TickUI();

	SceneUpdateGroupNode* NewGroup();
	void DeleteGroup(SceneUpdateGroupNode* group);
	void AttachGroup(SceneUpdateGroupNode* group);
	void DetachGroup(SceneUpdateGroupNode* group);
};