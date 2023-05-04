#include "app/SceneNode.h"
#include "app/Update.h"

SceneNode::SceneNode(Application* app, const char* name)
	: event       (&bus)
	, app         (app)
	, name        (name)
	, timeAcc     (0.f)
	, inDebugMode (false)
{
	entities.OnAdd   <Rigidbody2D>([this](Entity e) { physics.Add(e); });
	entities.OnRemove<Rigidbody2D>([this](Entity e) { physics.Remove(e); });
    
	entities.OnAdd<Transform2D>([this](Entity e)
    {
        if (e.Has<Rigidbody2D>())
            e.Get<Rigidbody2D>().SetTransform(e.Get<Transform2D>());
    });
}

SceneNode::~SceneNode()
{
	bus.DetachFromParent();

	for (SystemBase* system : update.GetOrderedList())
	{
		if (system->GetState() > SYSTEM_ATTACHED)
			system->_OnDetach();

		if (system->GetState() > SYSTEM_INIT)
			system->_Dnit();
	}
}

void SceneNode::_Tick(float deltaTime, float fixedTime)
{
	std::vector<SystemBase*> systems = update.GetOrderedList();

	// init or attach nodes

	for (SystemBase* system : systems)
	{
		if (system->GetState() == SYSTEM_CREATED)
			system->_Init(this);

		if (system->GetState() == SYSTEM_INIT)
			system->_OnAttach();
	}

	timeAcc += deltaTime;

	while (timeAcc > fixedTime)
	{
		// should subtract the fixed time, but that causes runaway frame drops
		//timeAcc -= fixedTime;
		timeAcc = 0;

		for (SystemBase* system : systems)
			system->_FixedUpdate();
	}

	for (SystemBase* system : systems)
		system->_Update();

	event.Execute();
	entities.ExecuteDeferdDeletions();
}

void SceneNode::_TickUI()
{
	update.Walk([this](SystemBase* s) { s->_UI(); });

	if (inDebugMode)
		update.Walk([this](SystemBase* s) { s->_Debug(); });
}