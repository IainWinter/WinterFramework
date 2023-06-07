#pragma once

#include "Rendering.h"
#include "util/Transform.h"
#include "ext/rendering/Camera.h"
#include "ext/rendering/Sprite.h"
#include "ext/rendering/Particle.h"
#include <unordered_map>

// could use major improvements !!!
// 
// 1.       don't realloc memory every frame
// 2.       clump draws by multiple textures (bind more than one at a time and add tex index to vertex info)
// 3. ABLE: write directly to buffer memory (maybe make a buffer a vector?)
// 4.       create all batches and upload at once, while draws are happening

struct BatchSpriteRenderer
{
public:
	BatchSpriteRenderer();
	//BatchSpriteRenderer(const char* vertexShader, const char* fragmentShader);

	void Begin();
	void Draw(const Camera& camera, bool mixTint = false);

	void SetZOffsetPerDraw(float zOffsetPerDraw);

	void SubmitColor(
		const Transform2D& transform,
		const Color& tint);

	void SubmitSprite(
		const Transform2D& transform,
		const Sprite& sprite);

	void SubmitParticle(
		const Transform2D& transform,
		const Particle& particle);

	void SubmitSprite(
		const Transform2D& transform, 
		const r<Texture>& texture,
		const vec2& uvOffset = vec2(0.f),
		const vec2& uvScale  = vec2(1.f),
		const Color& tint    = Color());
	
private:
	ShaderProgram m_program;

	// for custom shaders
	const char* m_vertexSource;
	const char* m_fragmentSource;

	Mesh m_quad;
	r<Texture> m_default;
	float z = 0;

	// Set to offset each draw by a small amount
	float zOffsetPerDraw = 0.0001f;

	void InitProgram();

	struct BatchData
	{
		Mesh quad;
		Buffer* uvs;
		Buffer* tints;
		Buffer* models;

		int count;

		BatchData();

		void add(const mat4& model, const vec4& uv, const vec4& tint);
		void finalize();
	};

	std::unordered_map<r<Texture>, BatchData> m_batches;

	void SetProgram(const mat4& proj, const mat4& view, bool mixTint);
	void DrawBatch(r<Texture> texture, BatchData& batch);
    

public:
    r<Texture> GetDefaultTexture() const;
};
