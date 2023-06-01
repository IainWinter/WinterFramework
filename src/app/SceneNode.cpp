#include "app/SceneNode.h"
#include "app/SceneUpdateGroupNode.h"
#include "app/Update.h"

SceneNode::SceneNode(Application* app)
	: event          (&bus)
	, app            (app)
	, timeAcc        (0.f)
	, inDebugMode    (false)
	, physicsRunning (true)
{
	entities.OnAdd   <Rigidbody2D>([this](Entity e) { physics.Add(e); });
	entities.OnRemove<Rigidbody2D>([this](Entity e) { physics.Remove(e); });
    
	entities.OnAdd<Transform2D>([this](Entity e)
    {
		if (Rigidbody2D* body = e.TryGet<Rigidbody2D>())
			body->SetTransform(e.Get<Transform2D>());
    });
}

SceneNode::~SceneNode()
{
	bus.DetachFromParent();

	for (SceneUpdateGroupNode* group : groups)
		group->Detach();

	for (SceneUpdateGroupNode* group : groups)
		group->Dnit();

	for (SceneUpdateGroupNode* group : groups)
		delete group;

	entities.Clear();
}

void SceneNode::Tick(float deltaTime, float fixedTime)
{
	event.Execute();
	entities.ExecuteDeferdDeletions();

	// init or attach nodes

	for (SceneUpdateGroupNode* group : groups)
		group->Init(this);

	for (SceneUpdateGroupNode* group : groups)
		group->Attach();

	// So far this seems to be the best way to also pause the physics
	// without putting the physics tick in a system
	if (physicsRunning)
	{
		timeAcc += deltaTime;

		while (timeAcc > fixedTime)
		{
			// should subtract the fixed time, but that causes runaway frame drops
			//timeAcc -= fixedTime;
			timeAcc = 0;

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
}

void SceneNode::DetachGroup(SceneUpdateGroupNode* group)
{
	auto itr = std::find(groups.begin(), groups.end(), group);

	if (itr == groups.end())
		return;

	groups.erase(itr);
	group->Detach();
}