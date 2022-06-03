#pragma once

#include "Entry.h"
#include "Leveling.h"
#include "Windowing.h"

struct TestSystem : System
{
	void Update()
	{
		for (auto [i] : Query<int>())
		{
			printf("%d\n", i);
		}
	}
};

struct EngineLoop
{
private:
	bool m_running = true;

public:
	void on(event_Shutdown& e) { m_running = false; }

	// Interface

	bool Running()
	{
		return m_running;
	}

	void Init()
	{ 
		events().attach<event_Shutdown>(this);
		return _Init();
	}

	void Dnit()
	{
		events().detach(this);
		return _Dnit(); 
	}

	void Tick()
	{
		return _Tick();
	}

protected:

	virtual void _Init() {};
	virtual void _Dnit() {};
	virtual void _Tick() = 0;
};

struct MyEngineLoop : EngineLoop
{
	void _Tick() override
	{
		r<Level> level = LevelManager::CurrentLevel();

		for (System* system : level->GetSystems()) system->Update();

		auto [window] = level->App()->GetModules<Window>();

		window.BeginImgui();
		for (System* system : level->GetSystems()) system->UI();
		window.EndImgui();

		window.EndFrame();
		window.PumpEvents();
	}
};

Application app; // could use shared ptr

void setup()
{
	app.AddModule<Window>(WindowConfig {"Leveling Test", 400, 400 }, &events());
	app.AddModule<LevelManager>(app);
	app.AddModule<MyEngineLoop>();

	r<Level> level = app.GetModule<LevelManager>().CreateLevel();
	level->AddSystem(TestSystem());
	level->CreateEntity().AddAll(1);
	level->CreateEntity().AddAll(2);
	level->CreateEntity().AddAll(3);
	level->CreateEntity().AddAll(4);

	EngineLoop& loop = app.GetModule<MyEngineLoop>();
	
	loop.Init();
	while (loop.Running()) loop.Tick();
	loop.Dnit();
}

bool loop()
{
	return false;
}