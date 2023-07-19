#include "app/SceneNode.h"
#include "app/SceneUpdateGroupNode.h"
#include "app/Update.h"

SceneNode::SceneNode(Application* app, v2EntitySceneData* data)
	: event          (&bus)
	, app            (app)
	, data           (data)
	, timeAcc        (0.f)
	, inDebugMode    (false)
	, physicsRunning (true)
{
	log_game("d~Scene node created");
}

SceneNode::~SceneNode()
{
	log_game("d~Scene node destroyed");

	bus.DetachFromParent();

	for (SceneUpdateGroupNode* group : groups)
		group->Detach();

	for (SceneUpdateGroupNode* group : groups)
		group->Dnit();

	for (SceneUpdateGroupNode* group : groups)
		delete group;

	// odd that detached groups are deleted out of order
	// of insertion

	for (SceneUpdateGroupNode* group : groupsDetached)
		group->Dnit();

	for (SceneUpdateGroupNode* group : groupsDetached)
		delete group;

	delete data;
	data = nullptr;
}

void SceneNode::Tick(float deltaTime, float fixedTime)
{
	// commit last frames changes to scene data
	// and populate views
	if (data)
		data->commit();

	// init or attach nodes

	for (SceneUpdateGroupNode* group : groups)
		group->Init(this);

	for (SceneUpdateGroupNode* group : groups)
		group->Attach();

	// So far this seems to be the best way to also pause the physics
	// without putting the physics tick in a system
	if (physicsRunning)
	{
		int maxitr = 2;
		int itr = 0;

		timeAcc += deltaTime;

		if (timeAcc > 0.5f) // if there is over a half second of physics to calculate, just skip it
			timeAcc = 0.f;

		while (timeAcc >= fixedTime)
		{
			// limit iterations
			if (itr > maxitr) break;
			itr += 1;

			// this is required to keep physics engine running at the same speed with vsync enabled
			timeAcc -= fixedTime;

			for (SceneUpdateGroupNode* group : groups)
				group->FixedUpdate();

			physics.Tick(fixedTime);
		}
	}

	for (SceneUpdateGroupNode* group : groups)
		group->Update();
}

void SceneNode::TickUI()
{
	for (SceneUpdateGroupNode* group : groups)
		group->UI();

	for (SceneUpdateGroupNode* group : groups)
		group->Debug();
}

SceneUpdateGroupNode* SceneNode::NewGroup()
{
	SceneUpdateGroupNode* group = new SceneUpdateGroupNode();

	groups.push_back(group);
	return group;
}

void SceneNode::DeleteGroup(SceneUpdateGroupNode* group)
{
	auto itr = std::find(groups.begin(), groups.end(), group);

	if (itr == groups.end())
		return;

	groups.erase(itr);
	delete group;
}

void SceneNode::AttachGroup(SceneUpdateGroupNode* group)
{
	auto itr = std::find(groups.begin(), groups.end(), group);

	if (itr != groups.end())
		return;

	groups.push_back(group);
	group->Attach();

	groupsDetached.erase(group);
}

void SceneNode::DetachGroup(SceneUpdateGroupNode* group)
{
	auto itr = std::find(groups.begin(), groups.end(), group);

	if (itr == groups.end())
		return;

	groups.erase(itr);
	group->Detach();

	groupsDetached.insert(group);
}