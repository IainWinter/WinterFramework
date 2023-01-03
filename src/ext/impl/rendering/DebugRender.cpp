#include "ext/rendering/DebugRender.h"

#ifdef ENABLE_DEBUG_RENDER

namespace Debug
{
	BatchLineRenderer debugLines;
    BatchSpriteRenderer debugSprites;

    void Begin()
    {
	    debugLines.Begin();
        debugSprites.Begin();
    }

    void End(const Camera& camera)
    {
	    debugLines.Draw(camera);
        debugSprites.Draw(camera);
    }

    void Line(const vec2& a, const vec2& b, const Color& color, float z)
    {
	    debugLines.SubmitLine(a, b, color, z);
    }

    void Ray(const vec2& a, const vec2& dist, const Color& color, float scale, float z)
    {
	    debugLines.SubmitLine(a, a + dist * scale, color, z);
    }

    void Box(const vec2& a, const vec2& b, const Color& color, float z)
    {
        vec2 center = (b + a) / 2.f;
        float radius = length(center);
        
        Box(center, radius, color, z);
    }

    void Box(const vec2& center, float radius, const Color& color, float z)
    {
        Transform2D transform;
        transform.position = vec3(center, 9.f);
        transform.scale = vec2(radius);
        transform.rotation = 0.f;
        transform.z = z;
        
        debugSprites.SubmitSprite(transform, color);
    }

    void Line(const vec3& a, const vec3& b, const Color& color, const Transform& transform)
    {
        debugLines.SubmitLine(a, b, color, color, transform);
    }
}
#endif
