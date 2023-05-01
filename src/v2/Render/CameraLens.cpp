#include "v2/Render/CameraLens.h"

CameraLens::CameraLens()
	: position (0, 0, 0)
	, rotation (1, 0, 0, 0)
	, height   (0)
	, aspect   (1)
	, near     (0)
	, far      (0)
	, ortho    (false)
{}

mat4 CameraLens::GetViewMatrix() const
{
	// maybe could cache
	return translate(toMat4(-rotation), -position);
}

mat4 CameraLens::GetProjectionMatrix() const
{
	float w = height * aspect;

	if (ortho)
		return glm::ortho(-w, w, -height, height, near, far);

	return glm::perspective(fovy, aspect, near, far);
}

CameraLens& CameraLens::SetView(Transform transform)
{
	position = transform.position;
	rotation = transform.rotation;
	return *this;
}

CameraLens& CameraLens::SetOrthographic(float height, float aspect, float near, float far)
{
	this->height = height;
	this->aspect = aspect;
	this->near = near;
	this->far = far;
	this->ortho = true;

	return *this;
}

CameraLens& CameraLens::SetPerspective(float fov, float aspect, float near, float far)
{
	this->fovy = fov;
	this->aspect = aspect;
	this->near = near;
	this->far = far;
	this->ortho = false;

	return *this;
}

vec2 CameraLens::ScreenSize() const
{
	return vec2(height * aspect, height);
}

vec2 CameraLens::ScreenToWorld2D(vec2 screen) const
{
	vec2 realDim = ScreenSize();
	vec2 pos = vec2(position.x, -position.y) + screen * realDim * 2.f - realDim;

	return vec2(pos.x, -pos.y);
}

vec2 CameraLens::World2DToScreen(vec2 world2D) const
{
	vec2 realDim = ScreenSize();
	vec2 pos = world2D - vec2(position);

	return (pos + realDim) / realDim / 2.f;
}

void EditorLikeCameraMovement(CameraLens& lens, const EditorLikeCameraMovementInput& input)
{
    if (input.leftShiftPressed)
    {
        vec3 localR = vec3(input.moveVel.x, 0,               0)  * lens.rotation;
        vec3 localF = vec3(0,               0, -input.moveVel.y) * lens.rotation;

        lens.position += (localR + localF) * 0.2f;
    }
    
    else
    if (input.rightMouseClicked) // panning
    {
        vec3 localR = vec3(input.panVel.x, 0,               0) * lens.rotation;
        vec3 localU = vec3(0,              -input.panVel.y, 0) * lens.rotation;

        lens.position -= (localR + localU) * 0.02f;
    }

    if (input.wheelVel) // moving forward
    {
        if (lens.ortho)
            lens.height = clamp(lens.height - input.wheelVel / 10.f, 0.01, 100.f);

        else
            lens.position -= vec3(0, 0, input.wheelVel) * lens.rotation * 0.08f;
    }

    if (input.middleMouseClicked || input.leftShiftPressed) // rotation
    {
        quat deltaP = angleAxis(input.panVel.x * input.deltaTime, vec3(0, 0, 1));
        quat deltaY = angleAxis(input.panVel.y * input.deltaTime, vec3(1, 0, 0) * lens.rotation);

        lens.rotation *= deltaY * deltaP;
    }
}
