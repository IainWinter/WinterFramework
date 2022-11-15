#include "impl/app/InternalSystems.h"

TransformUpdate::TransformUpdate()                       { SetName("Transform Update"); }
PhysicsInterpolationUpdate::PhysicsInterpolationUpdate() { SetName("Physics interpolation update"); }

void TransformUpdate::Update()
{
	for (auto [transform] : Query<Transform2D>())
	{
		transform.UpdateLastFrameData();
	}
}

void PhysicsInterpolationUpdate::Update()
{
	m_acc += Time::DeltaTime();
	
	float ratio = Time::FixedTime() == 0.f 
		? m_lastRatio 
		: clamp(m_acc / Time::FixedTime(), 0.f, 1.f);

	m_lastRatio = ratio;

	for (auto [transform, body] : Query<Transform2D, Rigidbody2D>())
	{
		transform.position.x = lerp(body.GetLastTransform().position.x, body.GetPosition().x, ratio);
		transform.position.y = lerp(body.GetLastTransform().position.y, body.GetPosition().y, ratio);
		transform.rotation   = lerp(body.GetLastTransform().rotation,   body.GetAngle(),      ratio);
	}
}

void PhysicsInterpolationUpdate::FixedUpdate()
{
	m_acc = 0;

	for (auto [body] : Query<Rigidbody2D>())
	{
		body.UpdateLastTransform();
	}

	for (auto [transform, body] : Query<Transform2D, Rigidbody2D>())
	{
		body.SetPosition(transform.position);
		body.SetAngle(transform.rotation);
	}
}

//void AudioUpdate::Update()
//{
//	Audio& audio = GetWorld()->GetApplication()->GetAudio();
//
//	for (auto [emitter] : Query<AudioEmitter>())
//	{
//		if (!emitter.IsPlaying())
//		{
//			emitter.Play();
//		}
//	}
//}
