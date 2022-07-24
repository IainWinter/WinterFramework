#pragma once

#include "app/System.h"

struct System_TransformInterpolation : SystemBase
{
	void Update() override
	{
		for (auto [transform] : Query<Transform2D>())
		{
			transform.UpdateLastFrameData();
		}
	}
};
