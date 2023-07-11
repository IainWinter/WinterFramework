#include "v2/Render/Particle.h"
#include "ext/rendering/ImportShader.h"

#include "util/error_check.h"
#include "util/filesystem.h"
#include "util/random.h"
#include "util/stl.h"

#include "ext/AssetStore.h"

#include "ext/serial/serial.h"
#include "ext/serial/serial_json.h"
#include <fstream>

void particle_RegisterMetaTypes()
{
	meta::describe<RandomBool>().name("RandomBool").member<&RandomBool::odds>("odds");
	meta::describe<RandomInt>().name("RandomInt").member<&RandomInt::min>("min").member<&RandomInt::max>("max");
	meta::describe<RandomFloat>().name("RandomFloat").member<&RandomFloat::min>("min").member<&RandomFloat::max>("max");
	meta::describe<RandomFloat2>().name("RandomFloat2").member<&RandomFloat2::min>("min").member<&RandomFloat2::max>("max");
	meta::describe<RandomFloat3>().name("RandomFloat3").member<&RandomFloat3::min>("min").member<&RandomFloat3::max>("max");
	meta::describe<RandomFloat4>().name("RandomFloat4").member<&RandomFloat4::min>("min").member<&RandomFloat4::max>("max");

	meta::describe<ParticleData>()
		.name("ParticleData")
		.member<&ParticleData::position>("position")
		.member<&ParticleData::rotation>("rotation")
		.member<&ParticleData::scale>("scale")
		.member<&ParticleData::tint>("tint")
		.member<&ParticleData::uvScale>("uvScale")
		.member<&ParticleData::uvOffset>("uvOffset")
		.member<&ParticleData::texture>("texture")
		.member<&ParticleData::userIndex>("userIndex")
		.member<&ParticleData::velocity>("velocity")
		.member<&ParticleData::damping>("damping")
		.member<&ParticleData::aVelocity>("aVelocity")
		.member<&ParticleData::aDamping>("aDamping")
		.member<&ParticleData::life>("life")
		.member<&ParticleData::enableScalingByLife>("enableScalingByLife")
		.member<&ParticleData::finalScale>("finalScale")
		.member<&ParticleData::factorScale>("factorScale")
		.member<&ParticleData::enableTintByLife>("enableTintByLife")
		.member<&ParticleData::finalTint>("finalTint")
		.member<&ParticleData::factorTint>("factorTint")
		.member<&ParticleData::additiveBlend>("additiveBlend")
		.member<&ParticleData::autoOrderZAroundOrigin>("autoOrderZAroundOrigin");

	meta::describe<ParticleSpawn>()
		.name("ParticleSpawn")
		.member<&ParticleSpawn::particle>("particle")
		.member<&ParticleSpawn::position>("position")
		.member<&ParticleSpawn::rotation>("rotation")
		.member<&ParticleSpawn::scale>("scale")
		.member<&ParticleSpawn::tint>("tint")
		.member<&ParticleSpawn::texture>("texture")
		.member<&ParticleSpawn::velocity>("velocity")
		.member<&ParticleSpawn::damping>("damping")
		.member<&ParticleSpawn::aVelocity>("aVelocity")
		.member<&ParticleSpawn::aDamping>("aDamping")
		.member<&ParticleSpawn::life>("life")
		.member<&ParticleSpawn::enableScalingByLife>("enableScalingByLife")
		.member<&ParticleSpawn::finalScale>("finalScale")
		.member<&ParticleSpawn::factorScale>("factorScale")
		.member<&ParticleSpawn::enableTintByLife>("enableTintByLife")
		.member<&ParticleSpawn::finalTint>("finalTint")
		.member<&ParticleSpawn::factorTint>("factorTint")
		.member<&ParticleSpawn::additiveBlend>("additiveBlend")
		.member<&ParticleSpawn::numberPerSpawn>("numberPerSpawn")
		.member<&ParticleSpawn::numberPerSecond>("numberPerSecond");
}

void particle_SaveSpawn(const ParticleSpawn& spawn, const std::string& filepath)
{
	std::filesystem::create_directories(std::filesystem::path(filepath).parent_path());

	std::ofstream out(filepath);
	if (out.is_open())
		json_writer(out).write(spawn);
}

ParticleSpawn particle_LoadSpawn(const std::string& filepath)
{
	ParticleSpawn spawn;

	std::ifstream in(filepath);
	if (in.is_open())
		json_reader(in).read(spawn);

	return spawn;
}

ParticleMesh::ParticleMesh(int fixedCount)
{
	m_particles = new ParticleData[fixedCount];
	m_fixedCount = fixedCount;

	// Create buffers

	vec2 positions[4] = { vec2(-1, -1), vec2(1, -1), vec2(1, 1), vec2(-1, 1) };
	vec2 uv[4] = { vec2(0,  0), vec2(1,  0), vec2(1, 1), vec2(0, 1) };
	int index[6] = { 0, 1, 2, 0, 2, 3 };

	GLuint positionVBO;
	gl(glGenBuffers(1, &positionVBO));
	gl(glBindBuffer(GL_ARRAY_BUFFER, positionVBO));
	gl(glBufferData(GL_ARRAY_BUFFER, sizeof(positions), positions, GL_STATIC_DRAW));

	GLuint uvVBO;
	gl(glGenBuffers(1, &uvVBO));
	gl(glBindBuffer(GL_ARRAY_BUFFER, uvVBO));
	gl(glBufferData(GL_ARRAY_BUFFER, sizeof(uv), uv, GL_STATIC_DRAW));

	GLuint indexEBO;
	gl(glGenBuffers(1, &indexEBO));
	gl(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexEBO));
	gl(glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(index), index, GL_STATIC_DRAW));

	gl(glGenBuffers(1, &m_particleInstVBO));
	gl(glBindBuffer(GL_ARRAY_BUFFER, m_particleInstVBO));
	gl(glBufferData(GL_ARRAY_BUFFER, sizeof(ParticleData) * fixedCount, m_particles, GL_DYNAMIC_DRAW));

	// Create buffer array 

	gl(glGenVertexArrays(1, &m_particlesVAO));
	gl(glBindVertexArray(m_particlesVAO));

	// set index buffer
	gl(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexEBO));

	// set fixed position at 0
	gl(glBindBuffer(GL_ARRAY_BUFFER, positionVBO));
	gl(glEnableVertexAttribArray(0));
	gl(glVertexAttribPointer(0, 2, GL_FLOAT, false, 0, 0));

	// set fixed uv at 1
	gl(glBindBuffer(GL_ARRAY_BUFFER, uvVBO));
	gl(glEnableVertexAttribArray(1));
	gl(glVertexAttribPointer(1, 2, GL_FLOAT, false, 0, 0));
	
	// set the particle instance data
	gl(glBindBuffer(GL_ARRAY_BUFFER, m_particleInstVBO));

	gl(glEnableVertexAttribArray(2));
	gl(glEnableVertexAttribArray(3));
	gl(glEnableVertexAttribArray(4));
	gl(glEnableVertexAttribArray(5));
	gl(glEnableVertexAttribArray(6));
	gl(glEnableVertexAttribArray(7));

	gl(glVertexAttribPointer(2, 3, GL_FLOAT, false, sizeof(ParticleData), (void*)offsetof(ParticleData, position)));
	gl(glVertexAttribPointer(3, 3, GL_FLOAT, false, sizeof(ParticleData), (void*)offsetof(ParticleData, rotation)));
	gl(glVertexAttribPointer(4, 2, GL_FLOAT, false, sizeof(ParticleData), (void*)offsetof(ParticleData, scale)));
	gl(glVertexAttribPointer(5, 4, GL_FLOAT, false, sizeof(ParticleData), (void*)offsetof(ParticleData, tint)));
	gl(glVertexAttribPointer(6, 2, GL_FLOAT, false, sizeof(ParticleData), (void*)offsetof(ParticleData, uvScale)));
	gl(glVertexAttribPointer(7, 2, GL_FLOAT, false, sizeof(ParticleData), (void*)offsetof(ParticleData, uvOffset)));

	gl(glVertexAttribDivisor(0, 0));
	gl(glVertexAttribDivisor(1, 0));
	gl(glVertexAttribDivisor(2, 1));
	gl(glVertexAttribDivisor(3, 1));
	gl(glVertexAttribDivisor(4, 1));
	gl(glVertexAttribDivisor(5, 1));
	gl(glVertexAttribDivisor(6, 1));
	gl(glVertexAttribDivisor(7, 1));
}

void ParticleMesh::Destroy()
{
	delete[] m_particles;
	gl(glDeleteVertexArrays(1, &m_particlesVAO));
}

ParticleData& ParticleMesh::Get(int i)
{
	return m_particles[i];
}

int ParticleMesh::Emit(const ParticleData& particle)
{
	if (count >= m_fixedCount)
		return -1;

	m_particles[count] = particle;
	count += 1;

	return count - 1;
}

void ParticleMesh::Update(float dt)
{
	for (int i = 0; i < count; i++)
	{
		ParticleData& data = m_particles[i];

		// pos / vel
		data.position += data.velocity * dt;
		data.rotation += data.aVelocity * dt;
		data.velocity *= clamp(1.f - dt * data.damping, 0.f, 1.f);
		data.aVelocity *= clamp(1.f - dt * data.aDamping, 0.f, 1.f);

		float lifeRatio = data.life / data.initialLife;

		// scale
		if (data.enableScalingByLife) {
			float ratio = 1.f - pow(lifeRatio, data.factorScale);
			data.scale = lerp(data.initialScale , data.finalScale, ratio);
		}

		// tint
		if (data.enableTintByLife) {
			float ratio = 1.f - pow(lifeRatio, data.factorTint);
			data.tint = lerp(data.initialTint, data.finalTint, ratio);
		}

		// life
		data.life -= dt;
		if (data.life < 0.f)
		{
			int newIndex = i;

			data = std::move(m_particles[count - 1]);
			count -= 1;
			i--;

			if (data.ifNotNullptrWriteMovedIndexHere)
				*data.ifNotNullptrWriteMovedIndexHere = newIndex;
		}
	}
}

void ParticleMesh::Draw()
{
	gl(glBindBuffer(GL_ARRAY_BUFFER, m_particleInstVBO));
	gl(glBufferSubData(GL_ARRAY_BUFFER, 0, count * sizeof(ParticleData), m_particles));

	gl(glBindVertexArray(m_particlesVAO));
	gl(glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr, count));
}

void ParticleMesh::SortZOrder(float zBase)
{
	float zDelta = 1.f / count;
	float z = zBase;
	for (int i = 0; i < count; i++)
	{
		if (!m_particles[i].autoOrderZAroundOrigin) // only do auto sorting if z = 0
			continue;

		m_particles[i].position.z = z;
		z += zDelta;
	}
}

ParticleSystem::ParticleSystem()
{
}

ParticleSystem::~ParticleSystem()
{
	m_additiveBlend.Destroy();
	m_noBlend.Destroy();
}

void ParticleSystem::Init()
{
	m_additiveBlend = ParticleMesh(500000);
	m_noBlend = ParticleMesh(100000);

	// load shader using old system
	m_shader = *ImportShader(_a("shaders/batch_particle.glsl").c_str());

	m_textureCache = TextureCache(2048, 2048, 4);

	RegTexture(Asset::LoadFromFile<Texture>(_a("sprites/star.png")));
	RegTexture(Asset::LoadFromFile<Texture>(_a("sprites/smoke1.png")));
	RegTexture(Asset::LoadFromFile<Texture>(_a("sprites/smoke2.png")));
	RegTexture(Asset::LoadFromFile<Texture>(_a("sprites/smoke3.png")));
	RegTexture(Asset::LoadFromFile<Texture>(_a("sprites/smoke4.png")));
	RegTexture(Asset::LoadFromFile<Texture>(_a("sprites/smoke5.png")));
	RegTexture(Asset::LoadFromFile<Texture>(_a("sprites/smoke6.png")));
}

int ParticleSystem::GetCount() const
{
	return m_noBlend.count + m_additiveBlend.count;
}

void ParticleSystem::SetScreen(vec2 min, vec2 max)
{
	m_min = min;
	m_max = max;
}

ParticleData& ParticleSystem::Get(int index, bool additive)
{
	return additive ? m_additiveBlend.Get(index) : m_noBlend.Get(index);
}

int ParticleSystem::Emit(const ParticleData& particle)
{
	bool outside = particle.position.x < m_min.x
				|| particle.position.y < m_min.y
				|| particle.position.x > m_max.x
				|| particle.position.y > m_max.y;

	if (outside)
		return -1;

	return EmitAllowOutsideBounds(particle);
}

int ParticleSystem::EmitAllowOutsideBounds(const ParticleData& particle)
{
	ParticleData p = particle; // this sucks...
	p.uvOffset = m_textureCacheImgs[particle.texture].offset;
	p.uvScale = m_textureCacheImgs[particle.texture].scale;
    p.initialLife = p.life;
    p.initialTint = p.tint;
    p.initialScale = p.scale;
    
	return p.additiveBlend
		? m_additiveBlend.Emit(p)
		: m_noBlend.Emit(p);
}

void ParticleSystem::EmitSpawn(const ParticleSpawn& spawn)
{
	int spawnCount = spawn.numberPerSpawn.get();

	for (int i = 0; i < spawnCount; i++)
	{
		ParticleData particle = spawn.particle;

		particle.position += spawn.position.get_circle();
		particle.rotation += spawn.rotation.get();
		particle.scale    += spawn.scale.get();

		particle.velocity  += spawn.velocity.get_circle();
		particle.damping   += spawn.damping.get();
		particle.aVelocity += spawn.aVelocity.get();
		particle.aDamping  += spawn.aDamping.get();

		//particle.tint *= spawn.tint.get();
		//particle.texture = spawn.texture.get();
		//particle.additiveBlend = spawn.additiveBlend.get();
		//particle.life *= spawn.life.get();
		//
		//particle.enableScalingByLife = spawn.enableScalingByLife.get();
		//particle.finalScale *= spawn.finalScale.get();
		//particle.factorScale *= spawn.factorScale.get();
		//
		//particle.enableTintByLife = spawn.enableTintByLife.get();
		//particle.finalTint *= spawn.finalTint.get();
		//particle.factorTint *= spawn.factorTint.get();

		Emit(particle);
	}
}

void ParticleSystem::EmitSpawnPerSecond(ParticleSpawn& spawn, float dt)
{
	if (spawn.numberPerSecond <= 0.0001f) // limit how many can spawn per second
		return;

	float delta = 1.f / spawn.numberPerSecond;
	
	spawn.timer += dt;
	if (spawn.timer > delta)
	{
		while (spawn.timer > delta)
		{
			spawn.timer -= delta;
			EmitSpawn(spawn);
		}
	}
}

void ParticleSystem::Update(float dt)
{
	m_additiveBlend.Update(dt);
	m_noBlend.Update(dt);
}

void ParticleSystem::Draw(const CameraLens& lens)
{
	if (m_oldCache) {
		m_oldCache = false;
		m_textureCache.SendToDevice();
	}

	m_shader.Use();
	m_shader.Set("view", lens.GetViewMatrix());
	m_shader.Set("proj", lens.GetProjectionMatrix());
	m_shader.Set("tex_sprite_sheet", m_textureCache.GetTexture());

	gl(glEnable(GL_DEPTH_TEST));

	gl(glEnable(GL_BLEND));
	gl(glBlendFunc(GL_SRC_ALPHA, GL_ONE));

	m_additiveBlend.SortZOrder(0.f);
	m_additiveBlend.Draw();

	gl(glEnable(GL_BLEND));
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	m_noBlend.SortZOrder(3.f);
	m_noBlend.Draw();
}

TextureCacheImg ParticleSystem::RegTexture(r<Texture> texture)
{
	TextureCacheImg img  = m_textureCache.Add((char*)texture->Pixels(), texture->Width(), texture->Height(), texture->Channels());

	m_textureCacheImgs[img.handle] = img;
	m_oldCache = true;

	return img;
}
