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

	void Line(const vec2& a, const vec2& b, const Color& color)
	{
		debugLines.SubmitLine(a, b, color);
	}

	void Ray(const vec2& a, const vec2& dist, const Color& color, float scale)
	{
		debugLines.SubmitLine(a, a + dist * scale, color);
	}

    void Box(const vec2& a, const vec2& b, const Color& color)
    {
        vec2 center = (b + a) / 2.f;
        float radius = length(center);
        
        Box(center, radius, color);
    }

    void Box(const vec2& center, float radius, const Color& color)
    {
        Transform2D transform;
        transform.position = vec3(center, 9.f);
        transform.scale = vec2(radius);
        transform.rotation = 0.f;
        
        debugSprites.SubmitSprite(transform, color);
    }
}
#endif
