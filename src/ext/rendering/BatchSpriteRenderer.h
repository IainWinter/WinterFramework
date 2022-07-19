#pragma once

#include "Rendering.h"
#include "GameRender.h"
#include "ext/rendering/Camera.h"
#include <unordered_map>

// could use major improvments !!!
// 
// 1. dont realloc memory every frame
// 2. clump draws by multiple textures (bind more than one at a time and add tex index to vertex info)
// 3. write directly to buffer memory (maybe make a buffer a vector?)
// 4. create all batches and upload at once, while draws are happening

struct BatchSpriteRenderer
{
private:
	float z = 0;

public:
	BatchSpriteRenderer()
	{
		InitProgram();
	}

	void Begin(Camera& camera)
	{
		SetProjection(camera.Projection());
		z = 0;
	}

	void SubmitSprite(const Transform2D& transform, const Color& tint)
	{
		SubmitSprite(transform, nullptr, vec2(0.f), vec2(0.f), tint);
	}

	void SubmitSprite(const Transform2D& transform, const r<Texture>& texture, const vec2& uvOffset, const vec2& uvScale, const Color& tint)
	{
		glm::mat4 world = transform.World();
		world[3][2] += z;
		z += .001f;

		BatchData& batch = m_batches[texture];
		batch.model.push_back(world);
		batch.uv   .push_back(vec4(uvOffset, uvScale));
		batch.tint .push_back(tint.as_v4());
	}

	void Draw()
	{
		for (auto& [texture, batch] : m_batches) DrawBatch(texture, batch);
		m_batches.clear(); // can have a much better scheme for keeping vector memory alive
	}

private:
	ShaderProgram m_program;
	Mesh m_quad;
	r<Texture> m_default;

	void InitProgram()
	{
		const char* source_vert = 
			"#version 330 core\n"

			"layout (location = 0) in vec2 pos;"
			"layout (location = 1) in vec2 uv;"

			"layout (location = 6) in vec4 uvOffsetAndScale;"
			"layout (location = 7) in vec4 tint;"
			"layout (location = 10) in mat4 model;" // reserves 8, 9, 10, 11

			"out vec2 TexCoords;"
			"out vec4 Tint;"

			"uniform mat4 projection;"

			"void main()"
			"{"
				"TexCoords = uv * uvOffsetAndScale.zw + uvOffsetAndScale.xy;"
				"Tint = tint;"
				"gl_Position = projection * model * vec4(pos, 0.0, 1.0);"
			"}";

		const char* source_frag = 
			"#version 330 core\n"

			"in vec2 TexCoords;"
			"in vec4 Tint;"

			"out vec4 color;"

			"uniform sampler2D sprite;"

			"void main()"
			"{"
				"vec4 spriteColor = Tint * texture(sprite, TexCoords);"
				"if (spriteColor.a == 0) discard;"
				"color = spriteColor;"
			"}";

		m_program.Add(ShaderProgram::sVertex,   source_vert);
		m_program.Add(ShaderProgram::sFragment, source_frag);

		r<Buffer> models = mkr<Buffer>(Buffer(0, 16, Buffer::_f32, DYNAMIC_HOST));

		m_quad = InitQuadMesh2D(Mesh(DYNAMIC_HOST))
			.Setup<vec4>(Mesh::aCustom_a1, 1)
			.Setup<vec4>(Mesh::aCustom_a2, 1)
			.Setup<mat4>(Mesh::aCustom_b1, 1);

		m_default = mkr<Texture>(_a("white.png")); // todo: grab from prefab
	}

	struct BatchData
	{
		std::vector<mat4> model;
		std::vector<vec4> uv;
		std::vector<vec4> tint;

		// assume that all lists are the same size, need to be seperate because mesh needs soa
		size_t size() const
		{
			return model.size();
		}
	};

	std::unordered_map<r<Texture>, BatchData> m_batches;

	void SetProjection(const mat4& projection)
	{
		m_program.Use();
		m_program.Set("projection", projection);
	}

	void DrawBatch(r<Texture> texture, BatchData& batch)
	{
		if (texture) m_program.Set("sprite", *texture);
		else         m_program.Set("sprite", *m_default);

		m_quad.Get(Mesh::aCustom_a1)->Set(batch.uv);
		m_quad.Get(Mesh::aCustom_a2)->Set(batch.tint);
		m_quad.Get(Mesh::aCustom_b1)->Set(batch.model.size(), batch.model.data());
		m_quad.DrawInstanced(batch.size());
	}
};