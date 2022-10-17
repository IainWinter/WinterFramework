#include "ext/rendering/DebugRender.h"

namespace Debug
{
	BatchLineRenderer debugLines;

	void Begin()
	{
		debugLines.Begin();
	}

	void End(const Camera& camera)
	{
		debugLines.Draw(camera);
	}

	void Line(const vec2& a, const vec2& b, const Color& color)
	{
		debugLines.SubmitLine(a, b, color);
	}

	void Ray(const vec2& a, const vec2& dist, const Color& color, float scale)
	{
		debugLines.SubmitLine(a, a + dist * scale, color);
	}
}