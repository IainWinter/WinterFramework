#pragma once

#include "app/System.h"

struct TransformUpdate : SystemBase
{
	void Update() override;
};

struct PhysicsInterpolationUpdate : SystemBase
{
	float m_acc = 0.f;
	float m_lastRatio = 0.f;

	void Update() override;
	void FixedUpdate() override;
};

//struct AudioUpdate : SystemBase
//{
//	void Update() override;
//};