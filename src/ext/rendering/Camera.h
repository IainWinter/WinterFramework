#pragma once

#include "Common.h"

struct Camera
{
	union
	{
		vec3 position;
		struct {
			float x, y, z;
		};
	};

	union {
		vec2 dimension;
		struct {
			float width, height;
		};
	};

	float near, far, aspect;
	
	// true  -> Projection returns an orthographic projection
	// false -> Projection returns a   perspective projection with height/aspect as the fovx/fovy
	bool is_ortho;

	// default, simple ortho
	Camera()
		: x(0), y(0), z(0), width(12), height(8), near(-10), far(10), aspect(1), is_ortho(true)
	{}

	// simple, ortho
	Camera(float width, float height, float depth)
		: x(0), y(0), z(0), width(width), height(height), near(-depth), far(depth), aspect(1), is_ortho(true)
	{}

	// simple persp
	Camera(float fov, float fovy, float near, float far)
		: x(0), y(0), z(0), width(fov), height(fovy), near(near), far(far), aspect(1), is_ortho(false)
	{}

	mat4 Projection() const
	{
		if (is_ortho)
		{
			float wr = width * aspect;
			return ortho(-wr, wr, -height, height, near, far);
		}

		else
		{
			return perspective(height, width * aspect, near, far);
		}
	}

	mat4 View(quat rotation = quat(1, 0, 0, 0)) const
	{
		return translate(toMat4(-rotation), -position);
	}

	vec2 ScreenSize() const
	{
		return vec2(width, height);
	}

	void SetPerspective(float fov, float fovy, float near, float far)
	{
		this->width = fov;
		this->height = fovy;
		this->near = near;
		this->far = far;
		this->is_ortho = false;
	}

	void SetOrthographoc(float width, float height, float depth)
	{
		this->width = width;
		this->height = height;
		this->near = -depth;
		this->far = depth;
		this->is_ortho = true;
	}
};
