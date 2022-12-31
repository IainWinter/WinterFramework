#pragma once

#include "Common.h"

struct Camera
{
	// this is just a transform...
	// kinda weird to have stored in the camera
	union {
		vec3 position;
		struct {
			float x, y, z;
		};
	};

	quat rotation;

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
		: position(0, 0, 0), rotation(1, 0, 0, 0), dimension(12, 8), near(-10), far(10), aspect(1), is_ortho(true)
	{}

	// simple, ortho
	Camera(float width, float height, float depth)
		: position(0, 0, 0), rotation(1, 0, 0, 0), dimension(width, height), near(-depth), far(depth), aspect(1), is_ortho(true)
	{}

	// simple persp
	Camera(float fov, float fovy, float near, float far)
		: position(0, 0, 0), rotation(1, 0, 0, 0), dimension(fov, fovy), near(near), far(far), aspect(1), is_ortho(false)
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

	mat4 View() const
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

	// assumes 2d editor mode, should just put in real rotation math
	vec2 ScreenToWorld2D(vec2 screen)
	{
		return vec2(position) + screen * dimension;
	}
};
