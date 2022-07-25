#pragma once

#include "app/System.h"
#include "ext/rendering/Camera.h"

struct InitMainMenu_System : SystemBase
{
	void Init()
	{
		AudioWorld& audio = GetWorld()->GetAudioWorld();

		audio.Load("Sounds/Master.strings.bank");
		audio.Load("Sounds/Master.bank");
	}
};