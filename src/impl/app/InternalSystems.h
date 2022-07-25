#pragma once

#include "app/System.h"

struct TransformUpdate : SystemBase
{
	void Update() override;
};

struct PhysicsInterpolationUpdate : SystemBase
{
	float m_acc = 0.f;

	void Update() override;
	void FixedUpdate() override;
};

struct AudioUpdate : SystemBase
{
	Audio* m_audio = nullptr;

	void LoadBank(AudioBankLoader& loader);
	void FreeBank(AudioBankLoader& loader);

	void Init() override;
	void Dnit() override;
	void Activate() override;
	void Deactivate() override;
	void Update() override;
};