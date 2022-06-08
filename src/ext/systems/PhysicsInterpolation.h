#pragma once

#include "Leveling.h"
#include "Physics.h"
#include "ext/Time.h"

struct PhysicsInterpolation : SystemBase
{
	float m_acc = 0.f;

	void Update() override
	{
		m_acc += Time::DeltaTime();
		float ratio = clamp(m_acc / Time::RawFixedTime(), 0.f, 1.f);

		for (auto [transform, body] : Query<Transform2D, Rigidbody2D>())
		{
			if (!body.InWorld()) continue;

			transform.x = lerp(body.LastTransform.x, body.GetPosition().x, ratio);
			transform.y = lerp(body.LastTransform.y, body.GetPosition().y, ratio);
			transform.r = lerp(body.LastTransform.r, body.GetAngle(), ratio);
		}
	}

	void FixedUpdate() override
	{
		m_acc = 0;

		for (auto [transform, body] : Query<Transform2D, Rigidbody2D>())
		{
			if (!body.InWorld()) continue;

			body.LastTransform.x = body.GetPosition().x;
			body.LastTransform.y = body.GetPosition().y;
			body.LastTransform.r = body.GetAngle();
		}
	}
};