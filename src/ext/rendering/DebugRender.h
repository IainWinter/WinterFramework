#pragma once

#include "Common.h"
#include "ext/rendering/BatchLineRenderer.h"

namespace Debug
{
	void Begin();
	void End(const Camera& camera);

	void Line(const vec2& a, const vec2& b,    const Color& color);
	void Ray (const vec2& a, const vec2& dist, const Color& color, float scale = 1.f);
}