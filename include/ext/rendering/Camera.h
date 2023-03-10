#pragma once

#include "Common.h"
#include "glm/gtx/quaternion.hpp"

struct Camera
{
	// this is just a transform...
	// kinda weird to have stored in the camera
	// but code gets annoying when only a camera is needed
	// should remove all these tho in the future
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
	Camera();

	// simple, ortho
	Camera(float width, float height, float depth);

	// simple persp
	Camera(float fov, float fovy, float near, float far);

	mat4 Projection() const;
	mat4 View() const;

	vec2 ScreenSize() const;

	void SetPerspective(float fov, float fovy, float near, float far);
	void SetOrthographoc(float width, float height, float depth);

	// assumes 2d editor mode, should just put in real rotation math
	vec2 ScreenToWorld2D(vec2 screen) const;
	vec2 World2DToScreen(vec2 world2D) const;
};
