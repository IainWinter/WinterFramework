#pragma once

#include "app/System.h"
#include "ext/rendering/Camera.h"

struct InitMainMenu_System : SystemBase
{
	void Init()
	{
		CreateEntity().Add<AudioBankLoader>()
			.AddBank("Sounds/Master.strings.bank")
			.AddBank("Sounds/Master.bank")
			.SetWhenToLoad(AudioBankLoader::ON_INIT)
			.SetWhenToFree(AudioBankLoader::ON_DNIT);
	}
};