#include "Prefabs.h"

// could put this in an assets file

std::unordered_map<std::string, r<ParticleEmitter>> emitters;

// could put in globals file

Entity CreateEntity()
{
	return LevelManager::CurrentLevel()->GetWorld()->Create();
}

template<typename _t>
_t& GetModule()
{
	return LevelManager::CurrentLevel()->GetApp()->GetModule<_t>();
}

ParticleEmitter GetPrefab_LaserEmitter()
{
	r<ParticleEmitter>& emitter = emitters["lazer.emitter"];
	
	if (!emitter)
	{
		emitter = mkr<ParticleEmitter>();

		Particle fireball = Particle(mkr<TextureAtlas>(mkr<Texture>(_a("diamond.png"))));
		fireball.repeatCount = 10;
		fireball.orignal.scale = vec2(.2, .1f);
		fireball.tints = {
			Color(255, 66, 66, 150),
			Color(220, 20, 20, 184)
		};

		emitter->AddSpawner(fireball, 1.f,
			[](Particle particle)
			{
					Entity entity = CreateEntity();
					entity.Add<Particle>(particle);
					entity.Add<Transform2D>(particle.orignal);

					entity.Add<ParticleShrinkWithAge>();

					Rigidbody2D&  body = GetModule<PhysicsWorld>().AddEntity(entity);
					body.SetAngularVelocity(get_randc(10.f));
					body.SetAngle(get_rand(w2PI));
					body.SetVelocity(get_randn(1.f));
					body.SetDamping(5.f);
			}
		);
	}

	return *emitter;
}
