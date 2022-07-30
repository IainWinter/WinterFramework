#pragma once

#include "app/System.h"
#include "Events.h"
#include "HighscoreRecord.h"
#include "sql/sqlite3.h"
#include "Data.h"

struct System_HighscoreStore : System<System_HighscoreStore>
{
	void OnAttach()
	{
		Attach<event_SubmitHighscore>();
		Attach<event_RequestHighscores>();
	}

	void on(event_SubmitHighscore& e)
	{
		AddHighscore(e.name.c_str(), e.score);
	}

	void on(event_RequestHighscores& e)
	{
		for (HighscoreRecord& record : GetAllHighscores())
		{
			SendToRoot(event_RespondHighscore{ record });
		}
	}
};