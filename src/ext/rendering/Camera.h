#pragma once

#include "Common.h"

struct Camera
{
	float x, y, w, h, z;

	Camera()
		: x(0), y(0), w(12), h(8), z(10)
	{}

	Camera(float x, float y, float w, float h, float z = 16.f)
		: x(x), y(y), w(w), h(h), z(z)
	{}

	mat4 Projection() const
	{
		mat4 camera = ortho(-w, w, -h, h, -z, z);
		camera = translate(camera, vec3(x, y, 0.f));

		return camera;
	}

	vec2 ScreenSize() const
	{
		return vec2(w, h);
	}
};