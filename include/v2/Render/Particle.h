#pragma once

#include "util/math.h"
#include "util/Color.h"

// use old system for now
#include "Rendering.h"

#include <array>

#include "v2/Render/CameraLens.h"
#include "v2/Render/TextureCache.h"



struct ParticleData
{
	vec3 position = vec3(0.f);
	vec3 rotation = vec3(0.f);
	vec2 scale = vec2(1.f);

	vec4 tint = vec4(1.f);
	
	vec2 uvScale = vec2(1.f);
	vec2 uvOffset = vec2(0.f);
	
	// this is an index of cached textures if in the particle system
	int texture = 1;

	// 
	// below has to be copied to GPU, but is only used for updating on CPU
	//

	// use this to index into other data arrays to effect the particle
	int userIndex = 0;
	int* ifNotNullptrWriteMovedIndexHere = nullptr;

	// would allow all this to be removed

	vec3 velocity = vec3(0.f);
	float damping = 0.f;

	vec3 aVelocity = vec3(0.f);
	float aDamping = 0.f;

	float life = 1.f;
	float initialLife = 0.f;

	bool enableScalingByLife = false;
	vec2 initialScale = vec2(1.f);
	vec2 finalScale = vec2(0.f);
	float factorScale = 1.f;

	bool enableTintByLife = false;
	vec4 initialTint = vec4(1, 1, 1, 1);
	vec4 finalTint = vec4(1, 1, 1, 0);
	float factorTint = 1.f;

	bool additiveBlend = false;
	bool autoOrderZAroundOrigin = true;
};

class ParticleMesh
{
private:
	GLuint m_particlesVAO = 0;
	GLuint m_particleInstVBO = 0;

	ParticleData* m_particles = nullptr;
	int m_fixedCount = 0;

public:
	int count = 0;

	ParticleMesh() = default;
	ParticleMesh(int fixedCount);

	// cant be bothered to make constructors
	void Destroy();

	ParticleData& Get(int i);

	int Emit(const ParticleData& particle);

	void Update(float dt);
	void Draw();

	void SortZOrder(float zBase);
};

// A particle system which stores a tightly packed list of 
// particles using a mesh for rendering
//
class ParticleSystem
{
public:
	ParticleSystem();
	~ParticleSystem();

	void Init();

	int GetCount() const;
	void SetScreen(vec2 min, vec2 max);

	// should remove index function because when particles are
	// deleted the index is ruined. Only if the index is the last particle

	ParticleData& Get(int index, bool additive);

	int Emit(const ParticleData& particle);
	int EmitAllowOutsideBounds(const ParticleData& particle);

	void Update(float dt);
	void Draw(const CameraLens& lens);

	TextureCacheImg RegTexture(r<Texture> texture);

private:
	ParticleMesh m_additiveBlend;
	ParticleMesh m_noBlend;

	ShaderProgram m_shader;

	bool m_oldCache;
	TextureCache m_textureCache;
	std::unordered_map<int, TextureCacheImg> m_textureCacheImgs;

	vec2 m_min;
	vec2 m_max;
};