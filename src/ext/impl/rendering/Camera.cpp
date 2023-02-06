#include "ext/rendering/Camera.h"

Camera::Camera()
	: position  (0, 0, 0)
	, rotation  (1, 0, 0, 0)
	, dimension (12, 8)
	, near      (-10)
	, far       (10)
	, aspect    (1)
	, is_ortho  (true)
{}

Camera::Camera(float width, float height, float depth)
	: position  (0, 0, 0)
	, rotation  (1, 0, 0, 0)
	, dimension (width, height)
	, near      (-depth)
	, far       (depth)
	, aspect    (1)
	, is_ortho  (true)
{}

Camera::Camera(float fov, float fovy, float near, float far)
	: position  (0, 0, 0)
	, rotation  (1, 0, 0, 0)
	, dimension (fov, fovy)
	, near      (near)
	, far       (far)
	, aspect    (1)
	, is_ortho  (false)
{}

mat4 Camera::Projection() const
{
	if (is_ortho)
	{
		float wr = width * aspect;
		return ortho(-wr, wr, -height, height, near, far);
	}

	return perspective(height, width * aspect, near, far);
}

mat4 Camera::View() const
{
	return translate(toMat4(-rotation), -position);
}

vec2 Camera::ScreenSize() const
{
	return vec2(width * aspect, height);
}

void Camera::SetPerspective(float fov, float fovy, float near, float far)
{
	this->width = fov;
	this->height = fovy;
	this->near = near;
	this->far = far;
	this->is_ortho = false;
}

void Camera::SetOrthographoc(float width, float height, float depth)
{
	this->width = width;
	this->height = height;
	this->near = -depth;
	this->far = depth;
	this->is_ortho = true;
}

vec2 Camera::ScreenToWorld2D(vec2 screen) const
{
	vec2 realDim = ScreenSize();
	vec2 pos = vec2(position.x, -position.y) + screen * realDim * 2.f - realDim;

	return vec2(pos.x, -pos.y);
}

vec2 Camera::World2DToScreen(vec2 world2D) const
{
	vec2 realDim = ScreenSize();
	vec2 pos = world2D - vec2(position);

	return (pos + realDim) / realDim / 2.f;
}