#pragma once

#include "util/Transform3D.h"

#undef near
#undef far

//	Defines the coordinate system for a camera by a view and projection matrix. In future
//	will have more properties of a lens.
//
class CameraLens
{
public:
	// A specialized transform to calculate the
	// view matrix. Scale is not used

	vec3 position;
	quat rotation;
	
	union { float fovy, height; };

	float aspect;
	float near;
	float far;

	// is this camera lens orthographic or perspective
	bool ortho;

public:
	CameraLens();

	mat4 GetViewMatrix() const;
	mat4 GetProjectionMatrix() const;

	CameraLens& SetView(Transform transform);

	CameraLens& SetOrthographic(float height, float aspect, float near, float far);
	CameraLens& SetPerspective(float fov, float aspect, float near, float far);

	// returns (aspect * height, height)
	vec2 ScreenSize() const;

	// assumes 2d, should just put in real rotation math
	vec2 ScreenToWorld2D(vec2 screen) const;
	vec2 World2DToScreen(vec2 world2D) const;
};

CameraLens lens_Orthographic(float height, float aspect, float near, float far);
CameraLens lens_Perspective(float fov, float aspect, float near, float far);

struct EditorLikeCameraMovementInput
{
	bool rightMouseClicked;
	bool middleMouseClicked;
    bool leftShiftPressed;
    
    vec2 moveVel;
    
	vec2 panVel;
	float wheelVel;

	float deltaTime;
};

void EditorLikeCameraMovement(CameraLens& lens, const EditorLikeCameraMovementInput& input);
