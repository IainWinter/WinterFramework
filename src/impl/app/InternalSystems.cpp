#include "impl/app/InternalSystems.h"
#include "ext/rendering/Particle.h"

TransformUpdate::TransformUpdate()                       { SetName("Transform Update"); }
PhysicsInterpolationUpdate::PhysicsInterpolationUpdate() { SetName("Physics Interpolation Update"); }
ParticleUpdate::ParticleUpdate()                         { SetName("Particle Update"); }

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

void ParticleUpdate::Update()
{
	// particle frame update

	for (auto [entity, particle] : QueryWithEntity<Particle>())
	{
		particle.TickFrame(Time::DeltaTime());

		if (particle.EndOfLife())
		{
			entity.Destroy();
		}
	}

	// engine effects

	for (auto [transform, particle, scaleWithAge] : Query<Transform2D, Particle, ParticleScaleWithAge>())
	{
		transform.scale = lerp(particle.original.scale, scaleWithAge.scale, particle.AgeLeft());
	}

	// emitter update

	for (auto [transform, emitter] : Query<Transform2D, ParticleEmitter>())
	{
		if (!emitter.enableAutoEmit) continue;
			
		emitter.currentTime += Time::DeltaTime();
		if (emitter.currentTime > emitter.timeBetweenSpawn)
		{
			emitter.currentTime = 0;

			std::vector<Particle> particles = emitter.EmitLine(
				transform.LastTransform().position, 
				transform.position,
				.1f
			);

			for (Particle& p : particles)
			{
				Entity e = CreateEntity();
				e.Add<Particle>(p);
				e.Add<Transform2D>(p.original)
					.SetZIndex(transform.z);

				const auto& func = emitter.spawners.at(p.m_emitterSpawnerIndex).onCreate;
				if (func) func(e);
			}
		}
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

void _AddInternalSystemsToWorld(World* world)
{
	world->CreateSystem<TransformUpdate>();
	world->CreateSystem<PhysicsInterpolationUpdate>();
	world->CreateSystem<ParticleUpdate>();
}
