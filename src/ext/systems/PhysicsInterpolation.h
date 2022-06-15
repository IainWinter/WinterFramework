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

			transform.position.x = lerp(body.LastTransform.position.x, body.GetPosition().x, ratio);
			transform.position.y = lerp(body.LastTransform.position.y, body.GetPosition().y, ratio);
			transform.rotation   = lerp(body.LastTransform.rotation,   body.GetAngle(),      ratio);
		}
	}

	void FixedUpdate() override
	{
		m_acc = 0;

		for (auto [transform, body] : Query<Transform2D, Rigidbody2D>())
		{
			if (!body.InWorld()) continue;

			body.LastTransform.position = body.GetPosition();
			body.LastTransform.rotation = body.GetAngle();
		}
	}
};
