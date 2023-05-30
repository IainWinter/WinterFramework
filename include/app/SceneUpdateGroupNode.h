#pragma once

#include "app/Update.h"

struct SceneUpdateGroupNode
{
	std::vector<SystemBase*> systems;

	~SceneUpdateGroupNode();

	void TakeOwnershipOfSystem(SystemBase* system);

	void Init(SceneNode* scene);
	void Dnit();

	void Attach();
	void Detach();

	void UI();
	void Debug();

	void Update();
	void FixedUpdate();
};