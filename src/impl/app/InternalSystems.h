#pragma once

#include "app/System.h"

struct TransformUpdate : SystemBase
{
    TransformUpdate();
    
	void Update() override;
};

struct PhysicsInterpolationUpdate : SystemBase
{
	float m_acc = 0.f;
	float m_lastRatio = 0.f;

    PhysicsInterpolationUpdate();
    
	void Update() override;
	void FixedUpdate() override;
};

struct ParticleUpdate : SystemBase
{
	void Update() override;
};

void _AddInternalSystemsToWorld(World* world);

//struct AudioUpdate : SystemBase
//{
//	void Update() override;
//};
