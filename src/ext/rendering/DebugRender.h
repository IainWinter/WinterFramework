#pragma once

#include "Common.h"
#include "ext/rendering/BatchLineRenderer.h"
#include "ext/rendering/BatchSpriteRenderer.h"

//#define ENABLE_DEBUG_RENDER

namespace Debug
{
#ifdef ENABLE_DEBUG_RENDER

	void Begin();
	void End(const Camera& camera); // add mix tint

	void Line(const vec2& a, const vec2& b,     const Color& color);
	void Ray (const vec2& a, const vec2& dist,  const Color& color, float scale = 1.f);
    void Box (const vec2& a, const vec2& b,     const Color& color);
    void Box (const vec2& center, float radius, const Color& color);

#else

    // these get optimized to nothing

    inline void Begin() {}
    inline void End(const Camera& camera) {}

    inline void Line(const vec2& a, const vec2& b,     const Color& color) {}
    inline void Ray (const vec2& a, const vec2& dist,  const Color& color, float scale = 1.f) {}
    inline void Box (const vec2& a, const vec2& b,     const Color& color) {}
    inline void Box (const vec2& center, float radius, const Color& color) {}

#endif
}
