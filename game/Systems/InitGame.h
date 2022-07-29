#pragma once

#include "app/System.h"
#include "Sand/Sand.h"

struct InitGame_System : System<InitGame_System>
{
	void Init()
	{
		PlayMusic();

		vec2 screenSize = GetWindow().Dimensions();
		vec2 cameraSize = vec2(16 * 2, 9 * 2);

		int cellsPerMeter = 18;

		CreateEntity().Add<SandWorld>(cellsPerMeter, cameraSize);
		CreateEntity().Add<Camera>(0, 0, cameraSize.x, cameraSize.y);

		CoordTranslation& coords = CreateEntity().Add<CoordTranslation>();
		coords.ScreenToWorld = cameraSize;
		coords.CellsToMeters = 1.f / cellsPerMeter;
	}

	void OnAttach()
	{
		Attach<event_Gameover>();
	}

	void on(event_Gameover& e)
	{
		TryStopMusic();
	}

	void Dnit()
	{
		TryStopMusic();
	}

private:

	int bg_music;

	void PlayMusic()
	{
		bg_music = PlaySound("event:/music_game_easy");

	}

	void TryStopMusic()
	{
		if (bg_music)
		{
			StopSound(bg_music);
			bg_music = 0;
		}
	}
};