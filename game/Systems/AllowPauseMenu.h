#pragma once

#include "Leveling.h"
#include "UI/PauseMenu.h"

struct System_AllowPauseMenu : System<System_AllowPauseMenu>
{
private:
	Order pause;
	float timescale;

public:

	void Init()
	{
		Attach<event_Input>();
	}

	void on(event_Input& e)
	{
		if (e.name == InputName::ESCAPE && e.state == 1)
		{
			if (pause) // pause is open 
			{
				LevelManager::CurrentLevel()->RemoveSystem(pause);
				pause = nullptr;
				Time::SetTimeScale(timescale);
			}

			else 
			{ 
				pause = LevelManager::CurrentLevel()->AddSystem(System_UI_PauseMenu());
				timescale = Time::TimeScale();
				Time::SetTimeScale(0.f);
			}
		}
	}
};