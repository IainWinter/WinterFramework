#include "app/SceneUpdateGroupNode.h"

SceneUpdateGroupNode::~SceneUpdateGroupNode()
{
	Detach();
	Dnit();

	for (SystemBase* system : systems)
		delete system;
}

void SceneUpdateGroupNode::TakeOwnershipOfSystem(SystemBase* system)
{
	systems.push_back(system);
}

void SceneUpdateGroupNode::Init(SceneNode* scene) 
{
	for (SystemBase* system : systems)
		if (system->GetState() == SYSTEM_CREATED)
			system->_Init(scene);
}

void SceneUpdateGroupNode::Dnit()
{
	for (SystemBase* system : systems)
		if (system->GetState() >= SYSTEM_INIT)
			system->_Dnit();
}

void SceneUpdateGroupNode::Attach()
{
	for (SystemBase* system : systems)
		if (system->GetState() == SYSTEM_INIT || system->GetState() == SYSTEM_DETACHED)
			system->_OnAttach();
}

void SceneUpdateGroupNode::Detach()
{
	for (SystemBase* system : systems)
		if (system->GetState() >= SYSTEM_ATTACHED)
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
