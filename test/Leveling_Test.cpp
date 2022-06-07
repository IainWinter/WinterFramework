#pragma once

#include "EngineLoop.h"
#include "Leveling.h"
#include "ext/Time.h"

struct TestSystem : System
{
	void Update()
	{
		for (auto [i] : Query<int>())
		{
			printf("%d\n", i);
		}
	}

	void UI()
	{
		ImGui::Begin("Test");
		ImGui::Text("a string");
		ImGui::End();
	}
};

struct MyGame : EngineLoop
{
	void _Init() override
	{
		LevelManager::CurrentLevel()->AddSystem(TestSystem());
	}
};

void setup()
{
	RunEngineLoop<MyGame>();
}