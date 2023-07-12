#pragma once

#include "ext/rendering/BatchLineRenderer.h"
#include "ext/rendering/BatchSpriteRenderer.h"

#define ENABLE_DEBUG_RENDER

namespace Debug
{
#ifdef ENABLE_DEBUG_RENDER

    void Init();
    void Dnit();

	void Begin();
	void End(const Camera& camera); // add mix tint

	void Line(const vec2& a, const vec2& b,     const Color& color, float z = 0.f);
	void Ray (const vec2& a, const vec2& dist,  const Color& color, float scale = 1.f, float z = 0.f);
    void Box (const vec2& a, const vec2& b,     const Color& color, float z = 0.f);
    void Box (const vec2& center, float radius, const Color& color, float z = 0.f);

    void Circle(const vec2& center, float radius, const Color& color);

    void Line(const vec3& a, const vec3& b, const Color& color, const Transform& transform = {});


#else

    // these get optimized to nothing

    inline void Init() {}
    inline void Dnit() {}

    inline void Begin() {}
    inline void End(const Camera& camera) {}

    inline void Line(const vec2& a, const vec2& b,     const Color& color, float z = 0.f) {}
    inline void Ray (const vec2& a, const vec2& dist,  const Color& color, float scale = 1.f, float z = 0.f) {}
    inline void Box (const vec2& a, const vec2& b,     const Color& color, float z = 0.f) {}
    inline void Box (const vec2& center, float radius, const Color& color, float z = 0.f) {}

    inline void Circle(const vec2& center, float radius, const Color& color) {}

    inline void Line(const vec3& a, const vec3& b, const Color& color, const Transform& transform = {}) {}

#endif
}
