#pragma once

#include "Rendering.h"
#include "ext/rendering/Camera.h"
#include "util/Transform.h"
#include "util/Transform3D.h"
#include <unordered_map>

// could use major improvments !!!
// 
// 1. see BatchSpriteRenderer most apply here

struct BatchLineRenderer
{
private:
	ShaderProgram m_program;
	Mesh m_mesh;

public:
	BatchLineRenderer();

	void Begin();

	void SubmitLine(const vec2& a, const vec2& b, const Color& color,  float z = 0.f);
	void SubmitLine(const vec2& a, const vec2& b, const Color& colorA, const Color& colorB, float z = 0.f);
	void SubmitLine(const vec2& a, const vec2& b, const Color& color,  const Transform2D& transform);
	void SubmitLine(const vec2& a, const vec2& b, const Color& colorA, const Color& colorB, const Transform2D& transform);

	void SubmitLine(const vec3& a, const vec3& b, const Color& colorA, const Color& colorB, const Transform& transform);

	void Draw(const Camera& camera);

private:
	void InitProgram();
};
