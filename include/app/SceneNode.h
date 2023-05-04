#pragma once

#include "app/UpdateTree.h"

#include "Entity.h"
#include "Physics.h"
#include "Event.h"

class Application;

struct SceneNode
{
	Application* app;

	EntityWorld entities;
	PhysicsWorld physics;
	EventBus bus;
	EventQueue event;

	UpdateTree update;
	float timeAcc;
	bool inDebugMode;

	const char* name;

	SceneNode(Application* app, const char* name);
	~SceneNode();

private:
	friend class Application;

private:
	void _Tick(float deltaTime, float fixedTime);
	void _TickUI();

private:
	// todo: 
	// The order is always fixed, so only enumerate tree when it has changed
	//std::vector<SystemBase*> updateCache;
};