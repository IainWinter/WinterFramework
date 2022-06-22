#pragma once

#include "Common.h"

struct Camera
{
	float x, y, w, h;

	Camera()
		: x(0), y(0), w(12), h(8) 
	{}

	Camera(float x, float y, float w, float h)
		: x(x), y(y), w(w), h(h)
	{}

	mat4 Projection() const
	{
		mat4 camera = ortho(-w, w, -h, h, -16.f, 16.f);
		camera = translate(camera, vec3(x, y, 0.f));

		return camera;
	}

	vec2 ScreenSize() const
	{
		return vec2(w, h);
	}
};