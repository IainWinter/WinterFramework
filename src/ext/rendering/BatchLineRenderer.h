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

	void SubmitLine(                              vec2& a, vec2& b, const Color& color);
	void SubmitLine(                              vec2& a, vec2& b, const Color& colorA, const Color& colorB);
	void SubmitLine(const Transform2D& transform, vec2& a, vec2& b, const Color& color);
	void SubmitLine(const Transform2D& transform, vec2& a, vec2& b, const Color& colorA, const Color& colorB);

	void Draw(const Camera& camera);

private:
	void InitProgram();
};