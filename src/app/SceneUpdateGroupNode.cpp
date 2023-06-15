#include "app/SceneUpdateGroupNode.h"

SceneUpdateGroupNode::~SceneUpdateGroupNode()
{
	Detach();
	Dnit();

	for (SystemBase* system : systems)
		delete system;
}

void SceneUpdateGroupNode::TakeOwnershipOfSystem(SystemBase* system, const char* name)
{
	system->_SetName(name);
	systems.push_back(system);
}

void SceneUpdateGroupNode::Init(SceneNode* scene) 
{
	if (state != SYSTEM_CREATED)
		return;

	state = SYSTEM_INIT;

	for (SystemBase* system : systems)
		system->_Init(scene);
}

void SceneUpdateGroupNode::Dnit()
{
	if (state != SYSTEM_DETACHED)
		return;

	state = SYSTEM_DNIT;

	for (SystemBase* system : systems)
		system->_Dnit();
}

void SceneUpdateGroupNode::Attach()
{
	if (state != SYSTEM_INIT && state != SYSTEM_DETACHED)
		return;

	state = SYSTEM_ATTACHED;

	for (SystemBase* system : systems)
		system->_OnAttach();
}

void SceneUpdateGroupNode::Detach()
{
	if (state != SYSTEM_ATTACHED)
		return;

	state = SYSTEM_DETACHED;

	for (SystemBase* system : systems)
		system->_OnDetach();
}

void SceneUpdateGroupNode::UI()
{
	for (SystemBase* system : systems)
		system->_UI();
}

void SceneUpdateGroupNode::Debug()
{
	for (SystemBase* system : systems)
		system->_Debug();
}

void SceneUpdateGroupNode::Update()
{
	for (SystemBase* system : systems)
		system->_Update();
}

void SceneUpdateGroupNode::FixedUpdate()
{
	for (SystemBase* system : systems)
		system->_FixedUpdate();
}