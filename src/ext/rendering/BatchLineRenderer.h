#pragma once

#include "Rendering.h"
#include "ext/rendering/Camera.h"
#include <unordered_map>

// could use major improvments !!!
// 
// 1. see BatchSpriteRenderer most apply here

struct BatchLineRenderer
{
private:
	ShaderProgram m_program;
	Mesh m_mesh;
	float z = 0;

public:
	BatchLineRenderer();

	void Begin();

	void SubmitLine(                              const vec2& a, const vec2& b, const Color& color,                       float z = 0.f);
	void SubmitLine(                              const vec2& a, const vec2& b, const Color& colorA, const Color& colorB, float z = 0.f);
	void SubmitLine(const Transform2D& transform, const vec2& a, const vec2& b, const Color& color);
	void SubmitLine(const Transform2D& transform, const vec2& a, const vec2& b, const Color& colorA, const Color& colorB);

	void Draw(const Camera& camera);

private:
	void InitProgram();
};
