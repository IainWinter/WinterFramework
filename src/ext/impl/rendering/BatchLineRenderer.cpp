#include "ext/rendering/BatchLineRenderer.h"

BatchLineRenderer::BatchLineRenderer()
{
	InitProgram();
}

void BatchLineRenderer::Begin()
{
	m_mesh.Clear();
}

void BatchLineRenderer::SubmitLine(const vec2& a, const vec2& b, const Color& color, float z)
{
    Transform2D t;
    t.z = z;
	SubmitLine(a, b, color, color, t);
}

void BatchLineRenderer::SubmitLine(const vec2& a, const vec2& b, const Color& colorA, const Color& colorB, float z)
{
    Transform2D t;
    t.z = z;
	SubmitLine(a, b, colorA, colorB, t);
}

void BatchLineRenderer::SubmitLine(const vec2& a, const vec2& b, const Color& color, const Transform2D& transform)
{
	SubmitLine(a, b, color, color, transform);
}

void BatchLineRenderer::SubmitLine(const vec2& a, const vec2& b, const Color& colorA, const Color& colorB, const Transform2D& transform)
{
	SubmitLine(vec3(a, 0.f), vec3(b, 0.f), colorA, colorB, transform);
}

void BatchLineRenderer::SubmitLine(const vec3& a, const vec3& b, const Color& colorA, const Color& colorB, const Transform2D& transform)
{
	glm::mat4 world = transform.World();
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
	m_program.Set("view", camera.View());
	m_program.Set("proj", camera.Projection());
	m_mesh.Draw(Mesh::tLines);
}

void BatchLineRenderer::InitProgram()
{
	const char* source_vert = 
		"#version 330 core\n"

		"layout (location = 0) in vec3 pos;"
		"layout (location = 5) in vec4 color;"
		"layout (location = 6) in mat4 model;"

		"out vec4 Color;"

		"uniform mat4 view;"
		"uniform mat4 proj;"

		"void main()"
		"{"
			"Color = color;"
			"gl_Position = proj * view * model * vec4(pos, 1.0);"
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
