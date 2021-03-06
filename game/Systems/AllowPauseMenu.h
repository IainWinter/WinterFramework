#pragma once

#include "app/System.h"
#include "UI/PauseMenu.h"

struct System_AllowPauseMenu : System<System_AllowPauseMenu>
{
private:
	SystemBase* pause;
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
			// this needs to send events on root to app

			//if (pause) // pause is open 
			//{
			//	LevelManager::CurrentLevel()->DestroySystem(pause);
			//	pause = nullptr;
			//	Time::SetTimeScale(timescale);
			//}

			//else 
			//{ 
			//	pause = LevelManager::CurrentLevel()->CreateSystem(System_UI_PauseMenu());
			//	timescale = Time::TimeScale();
			//	Time::SetTimeScale(0.f);
			//}
		}
	}
};