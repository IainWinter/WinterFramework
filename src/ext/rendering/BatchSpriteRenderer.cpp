#include "ext/rendering/BatchSpriteRenderer.h"

BatchSpriteRenderer::BatchSpriteRenderer()
{
	InitProgram();
}

//BatchSpriteRenderer::BatchSpriteRenderer(const char* vertexShader, const char* fragmentShader)
//{
//}

void BatchSpriteRenderer::Begin()
{
	z = 0;// -camera.z / 2 + 0.01f;
}

void BatchSpriteRenderer::Draw(const Camera& camera, bool mixTint)
{
	SetProgram(camera.Projection(), camera.View(), mixTint);

	for (auto& [texture, batch] : m_batches) DrawBatch(texture, batch);
	m_batches.clear(); // can have a much better scheme for keeping vector memory alive
}

void BatchSpriteRenderer::SetZOffsetPerDraw(float zOffsetPerDraw)
{
	this->zOffsetPerDraw = zOffsetPerDraw;
}

void BatchSpriteRenderer::SubmitColor(const Transform2D& transform, const Color& tint)
{
	SubmitSprite(transform, nullptr, vec2(0.f), vec2(0.f), tint);
}

void BatchSpriteRenderer::SubmitSprite(const Transform2D& transform, const Sprite& sprite)
{
	SubmitSprite(transform, sprite.source, sprite.uvOffset, sprite.uvScale, sprite.tint);
}

void BatchSpriteRenderer::SubmitParticle(const Transform2D& transform, const Particle& particle)
{
	if (particle.HasAtlas())
	{
		const TextureAtlas::Bounds& bounds = particle.GetCurrentFrameUV();
		SubmitSprite(transform, particle.atlas->source, bounds.uvOffset, bounds.uvScale, particle.GetTint());
	}

	else
	{
		SubmitSprite(transform, nullptr, vec2(0.f), vec2(1.f), particle.GetTint());
	}
}

void BatchSpriteRenderer::SubmitSprite(const Transform2D& transform, const r<Texture>& texture, const vec2& uvOffset, const vec2& uvScale, const Color& tint)
{
	// could compress to: 
	// uv (4 floats) 
	// model (scale 2 float, pos 3 floats, rot 1 float) 6 floats
	// tint (1 uint32) 

	mat4 world = transform.World();
	world[3][2] += z;

	// add a little offset to stop z fighting
	// gets a little wonky with many particles
	z += zOffsetPerDraw;

	vec4 uv = vec4(uvOffset, uvScale);

	m_batches[texture].add(world, uv, tint.as_v4());
}

void BatchSpriteRenderer::InitProgram()
{
	const char* source_vert = 
		"#version 330 core\n"

		"layout (location = 0) in vec2 pos;"
		"layout (location = 1) in vec2 uv;"

		"layout (location = 6) in vec4 uvOffsetAndScale;"
		"layout (location = 7) in vec4 tint;"
		"layout (location = 10) in mat4 model;"

		"out vec2 TexCoords;"
		"out vec4 Tint;"

		"uniform mat4 view;"
		"uniform mat4 proj;"

		"void main()"
		"{"
			"TexCoords = uv * uvOffsetAndScale.zw + uvOffsetAndScale.xy;"
			"Tint = tint;"
			"gl_Position = proj * view * model * vec4(pos, 0.0, 1.0);"
		"}";

	const char* source_frag = 
		"#version 330 core\n"

		"in vec2 TexCoords;"
		"in vec4 Tint;"

		"out vec4 color;"

		"uniform sampler2D sprite;"
		"uniform float mixTint = 0.f;"

		"void main()"
		"{"
			"vec4 textureColor = texture(sprite, TexCoords);"

			"if (mixTint == 1.f) {"
			"	textureColor.rgb = mix(textureColor.rgb, Tint.rgb, Tint.a);"
			"}"
			"else {"
			"	textureColor *= Tint;"
			"}"
			
			"if (textureColor.a == 0) {"
				"discard;"
			"}"
				
			"color = textureColor;"
		"}";

	m_program.Add(ShaderProgram::sVertex,   source_vert);
	m_program.Add(ShaderProgram::sFragment, source_frag);

	m_default = mkr<Texture>(Texture(5, 5, Texture::uRGBA));
	m_default->ClearHost(Color(255, 255, 255, 255));
}

void BatchSpriteRenderer::SetProgram(const mat4& proj, const mat4& view, bool mixTint)
{
	m_program.Use();
	m_program.Set("proj", proj);
	m_program.Set("view", view);
	m_program.Set("mixTint", (float)mixTint);
}

void BatchSpriteRenderer::DrawBatch(r<Texture> texture, BatchData& batch)
{
	if (texture) m_program.Set("sprite", *texture);
	else         m_program.Set("sprite", *m_default);

	batch.finalize();
}

r<Texture> BatchSpriteRenderer::GetDefaultTexture() const
{
    return m_default;
}

BatchSpriteRenderer::BatchData::BatchData()
{
	quad = Mesh(DYNAMIC_HOST)
		//.Add<vec2>(Mesh::aPosition,     { vec2(-1, -1), vec2(1, -1), vec2(1, 1), vec2(-1, 1) })
		.Add<vec2>(Mesh::aPosition,     { vec2(-0.5f, -0.5f), vec2(0.5f, -0.5f), vec2(0.5f, 0.5f), vec2(-0.5f, 0.5f) })
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

void BatchSpriteRenderer::BatchData::add(const mat4& model, const vec4& uv, const vec4& tint)
{
	uvs   ->Push(uv);
	tints ->Push(tint);
	models->Push(model);

	count += 1;
}

void BatchSpriteRenderer::BatchData::finalize()
{
	quad.MarkForUpdate();
	quad.DrawInstanced(count);

	uvs   ->Resize(count);
	tints ->Resize(count);
	models->Resize(count);

	count = 0;
}