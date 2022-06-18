#include "Sand/Render/DrawCollisionInfo.h"

SandCollisionInfoRenderer::SandCollisionInfoRenderer()
{
	m_shader = mkr<ShaderProgram>();
	m_quad   = mkr<Mesh>();

	m_quad->Add<vec2>(Mesh::aPosition, { vec2(-1, -1), vec2(1, -1), vec2(1, 1), vec2(-1, 1) });
	m_quad->Add<vec2>(Mesh::aTextureCoord, { vec2(0, 0), vec2(1, 0), vec2(1, 1), vec2(0, 1) });
	m_quad->Add<int>(Mesh::aIndexBuffer, { 0, 1, 2, 0, 2, 3});

	const char* source_vert =
		"#version 330 core\n"
		"layout (location = 0) in vec2 pos;"
		"layout (location = 1) in vec2 uv;"

		"out vec2 TexCoords;"

		"uniform mat4 model;"
		"uniform mat4 projection;"

		"void main()"
		"{"
			"TexCoords = uv;"
			"gl_Position = projection * model * vec4(pos, 0.0, 1.0);"
		"}";

	const char* source_frag =
		"#version 330 core\n"
		"in vec2 TexCoords;"

		"out ivec4 spriteId;" // sprite (x, y) (entity index), (alpha for if its even there)

		"uniform sampler2D colliderMask;"
		"uniform vec2 spriteSize;"
		"uniform int spriteIndex;"

		"void main()"
		"{"
			"vec4 mask = texture(colliderMask, TexCoords);"
			"if (mask.a > .7) mask.a = 1.f;" // round up for health thing

			"if (mask.a == 0) discard;"

			"spriteId = ivec4(TexCoords * spriteSize, spriteIndex, 1);"
		"}";

	m_shader->Add(ShaderProgram::sVertex, source_vert);
	m_shader->Add(ShaderProgram::sFragment, source_frag);
}

void SandCollisionInfoRenderer::Begin(Camera& camera, r<Target> target)
{
	if (target) target->Use();
	else Target::UseDefault();

	m_shader->Use();
	m_shader->Set("projection", camera.Projection());

	gl(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL));
	gl(glClearColor(0, 0, 0, 0));
	gl(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
}

void SandCollisionInfoRenderer::DrawCollisionInfo(const Transform2D& transform, SandSprite& sprite, int spriteIndex)
{
	Texture& mask = sprite.Get();

	m_shader->Set("model", transform.World());
	m_shader->Set("colliderMask", mask);
	m_shader->Set("spriteSize", vec2(mask.Width(), mask.Height()));
	m_shader->Set("spriteIndex", spriteIndex);

	m_quad->Draw();
}