#pragma once

#include <utility>
#include <stddef.h>
#include "Log.h"

namespace Time
{
	void UpdateTime();

	void SetFixedTime(float duration);
	void SetTimeScale(float duration);

	size_t Ticks();

	float TimeScale();
	float FixedTime();

	float TotalTime();
	float DeltaTime();
	
	float RawTotalTime();
	float RawDeltaTime();
	float RawFixedTime();

	float DeltaTimeNow();
	float TotalTimeNow();
}