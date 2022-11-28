#pragma once

#include "Rendering.h"
#include "ext/rendering/Camera.h"
#include "ext/rendering/Sprite.h"
#include <unordered_map>

// could use major improvments !!!
// 
// 1.       dont realloc memory every frame
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
	void Draw(const Camera& camera, const quat& rot, bool mixTint = false);
    
	void SubmitSprite(
		const Transform2D& transform,
		const Color& tint);

	void SubmitSprite(
		const Transform2D& transform,
		const Sprite& sprite);

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

	void InitProgram();

	struct BatchData
	{
		Mesh quad;
		Buffer* uvs;
		Buffer* tints;
		Buffer* models;

		int count;

		BatchData()
		{
			quad = Mesh(DYNAMIC_HOST)
				.Add<vec2>(Mesh::aPosition,     { vec2(-1, -1), vec2(1, -1), vec2(1, 1), vec2(-1, 1) })
				.Add<vec2>(Mesh::aTextureCoord, { vec2( 0,  0), vec2(1,  0), vec2(1, 1), vec2( 0, 1) })
				.Add<int> (Mesh::aIndexBuffer,  { 0, 1, 2, 0, 2, 3 })

				.Add<vec4>(Mesh::aCustom_a1, 1)
				.Add<vec4>(Mesh::aCustom_a2, 1)
				.Add<mat4>(Mesh::aCustom_b1, 1);

			uvs    = quad.Get(Mesh::aCustom_a1).get();
			tints  = quad.Get(Mesh::aCustom_a2).get();
			models = quad.Get(Mesh::aCustom_b1).get();

			count = 0;
		}

		void add(const mat4& model, const vec4& uv, const vec4& tint)
		{
			uvs   ->Push(uv);
			tints ->Push(tint);
			models->Push(model);

			count += 1;
		}

		void finalize()
		{
			quad.MarkForUpdate();
			quad.DrawInstanced(count);

			uvs   ->Resize(count);
			tints ->Resize(count);
			models->Resize(count);

			count = 0;
		}
	};

	std::unordered_map<r<Texture>, BatchData> m_batches;

	void SetProgram(const mat4& proj, const mat4& view, bool mixTint);
	void DrawBatch(r<Texture> texture, BatchData& batch);
    
public:
    r<Texture> GetDefaultTexture() const;
};
