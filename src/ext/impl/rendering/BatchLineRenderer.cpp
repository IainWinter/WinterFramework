#include "ext/rendering/BatchLineRenderer.h"

BatchLineRenderer::BatchLineRenderer()
{
	InitProgram();
}

void BatchLineRenderer::Begin()
{
	z = 0;
	m_mesh.Clear();
}

void BatchLineRenderer::SubmitLine(const vec2& a, const vec2& b, const Color& color, float z)
{
    Transform2D t;
    t.z = z;
	SubmitLine(t, a, b, color, color);
}

void BatchLineRenderer::SubmitLine(const vec2& a, const vec2& b, const Color& colorA, const Color& colorB, float z)
{
    Transform2D t;
    t.z = z;
	SubmitLine(t, a, b, colorA, colorB);
}

void BatchLineRenderer::SubmitLine(const Transform2D& transform, const vec2& a, const vec2& b, const Color& color)
{
	SubmitLine(transform, a, b, color, color);
}

void BatchLineRenderer::SubmitLine(const Transform2D& transform, const vec2& a, const vec2& b, const Color& colorA, const Color& colorB)
{
	glm::mat4 world = transform.World();
	world[3][2] += z;
	z += .001f;

	vec4 ca = colorA.as_v4();
	vec4 cb = colorB.as_v4();

	m_mesh.Get(Mesh::aCustom_a1)->Push(1, &world); // need to copy for each point
	m_mesh.Get(Mesh::aCustom_a1)->Push(1, &world);
	m_mesh.Get(Mesh::aPosition) ->Push(1, &a);
	m_mesh.Get(Mesh::aPosition) ->Push(1, &b);
	m_mesh.Get(Mesh::aColor)    ->Push(1, &ca);
	m_mesh.Get(Mesh::aColor)    ->Push(1, &cb);
}

void BatchLineRenderer::Draw(const Camera& camera)
{
	m_program.Use();
	m_program.Set("projection", camera.Projection());
	m_mesh.Draw(Mesh::tLines);
}

void BatchLineRenderer::InitProgram()
{
	const char* source_vert = 
		"#version 330 core\n"

		"layout (location = 0) in vec2 pos;"
		"layout (location = 5) in vec4 color;"
		"layout (location = 6) in mat4 model;"

		"out vec4 Color;"

		"uniform mat4 projection;"

		"void main()"
		"{"
			"Color = color;"
			"gl_Position = projection * model * vec4(pos, 0.0, 1.0);"
		"}";

	const char* source_frag = 
		"#version 330 core\n"

		"in vec4 Color;"

		"out vec4 color;"

		"void main()"
		"{"
			"if (Color.a == 0) {"
				"discard;"
			"}"
				
			"color = Color;"
		"}";

	m_program.Add(ShaderProgram::sVertex,   source_vert);
	m_program.Add(ShaderProgram::sFragment, source_frag);

	m_mesh = Mesh(DYNAMIC_HOST)
		.Add<vec2>(Mesh::aPosition)
		.Add<vec4>(Mesh::aColor)
		.Add<mat4>(Mesh::aCustom_a1);
}
