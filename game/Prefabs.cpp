#include "Prefabs.h"

// could put this in an assets file

std::unordered_map<std::string, r<ParticleEmitter>> emitters;
std::unordered_map<std::string, r<Texture>> textures;

void ClearPrefabs() // cleanup before context is destroied
{
	emitters.clear();
	textures.clear();
}

// could put in globals file

ParticleEmitter GetPrefab_BulletEmitter()
{
	r<ParticleEmitter>& emitter = emitters["bullet.emitter"];
	
	if (!emitter)
	{
		emitter = mkr<ParticleEmitter>();

		Particle bullet = Particle(mkr<TextureAtlas>(mkr<Texture>(_a("diamond.png"))));
		bullet.original.scale = vec2(.05, .05);
		bullet.tints = {
			Color(255, 200, 20, 255)
		};

		emitter->AddSpawner(bullet, 1.f,
			[](Entity entity)
			{
				entity.Add<Rigidbody2D>()
					.SetAngularVelocity(get_randc(3.f))
					.SetAngle(get_rand(w2PI))
					.SetVelocity(get_randn(1.f))
					.SetDamping(5.f);
			}
		);
	}

	return *emitter;
}

ParticleEmitter GetPrefab_LaserEmitter()
{
	r<ParticleEmitter>& emitter = emitters["lazer.emitter"];
	
	if (!emitter)
	{
		emitter = mkr<ParticleEmitter>();

		Particle fireball = Particle(mkr<TextureAtlas>(mkr<Texture>(_a("diamond.png"))));
		fireball.repeatCount = 10;
		fireball.original.scale = vec2(.2, .1f);
		fireball.tints = {
			Color(255, 66, 66, 150),
			Color(220, 20, 20, 184)
		};

		emitter->AddSpawner(fireball, 1.f,
			[](Entity entity)
			{
				entity.Add<ParticleShrinkWithAge>();

				entity.Add<Rigidbody2D>()
					.SetAngularVelocity(get_randc(10.f))
					.SetAngle(get_rand(w2PI))
					.SetVelocity(get_randn(1.f))
					.SetDamping(5.f);
			}
		);
	}

	return *emitter;
}

ParticleEmitter GetPrefab_FuelShotEmitter()
{
	r<ParticleEmitter>& emitter = emitters["fuel_shot.emitter"];
	
	if (!emitter)
	{
		emitter = mkr<ParticleEmitter>();

		Particle fireball = Particle(mkr<TextureAtlas>(mkr<Texture>(_a("diamond.png"))));
		fireball.repeatCount = 10;
		fireball.original.scale = vec2(1, .5f);
		fireball.tints = {
			Color( 10, 147, 255,   0),
			Color(244,  86,  12, 153),
			Color(255,  28,   0, 184)
		};

		emitter->AddSpawner(fireball, 1.f,
			[](Entity entity)
			{
				entity.Add<ParticleShrinkWithAge>();

				entity.Add<Rigidbody2D>()
					.SetAngularVelocity(get_randc(10.f))
					.SetAngle(get_rand(w2PI))
					.SetVelocity(get_randn(5.f))
					.SetDamping(5.f);
			});
	}

	return *emitter;
}

ParticleEmitter GetPrefab_LightningEmitter()
{
	r<ParticleEmitter>& emitter = emitters["lightning.emitter"];
	
	if (!emitter)
	{
		emitter = mkr<ParticleEmitter>();

		Particle fireball = Particle(mkr<TextureAtlas>(mkr<Texture>(_a("diamond.png"))));
		fireball.repeatCount = 3;
		fireball.original.scale = vec2(.3, .1f);
		fireball.tints = {
			Color(191, 240, 255, 100),
			Color( 48, 207, 255, 153),
			Color( 41, 159, 255, 184),
			Color( 41, 84,  255,   0)
		};

		emitter->AddSpawner(fireball, 1.f,
			[](Entity entity)
			{
				entity.Add<ParticleShrinkWithAge>();

				entity.Add<Rigidbody2D>()
					.SetAngularVelocity(get_randc(10.f))
					.SetAngle(get_rand(w2PI))
					.SetVelocity(get_randn(5.f))
					.SetDamping(5.f);
			});
	}

	return *emitter;
}

r<Texture> GetPrefab_Texture(const std::string& str, bool loadAsStatic)
{
	r<Texture>& texture = textures[str];

	if (!texture)
	{
		texture = mkr<Texture>(_a(str), loadAsStatic);
		if (texture->Pixels() == nullptr)
		{
			textures.erase(str); // delete if fails to load
		}
	}

	return texture;
}

