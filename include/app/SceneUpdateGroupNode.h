#pragma once

#include "app/Update.h"

struct SceneUpdateGroupNode
{
	std::vector<SystemBase*> systems;
	SystemState state;

	~SceneUpdateGroupNode();

	void TakeOwnershipOfSystem(SystemBase* system, const char* name);

	void Init(SceneNode* scene);
	void Dnit();

	void Attach();
	void Detach();

	void UI();
	void Debug();

	void Update();
	void FixedUpdate();
};