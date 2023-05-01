#include "v2/Render/Camera.h"

Camera_New& Camera_New::SetLens(const CameraLens& lens)
{
	this->lens = lens;
	return *this;
}

Camera_New& Camera_New::SetPass(a<RenderPass> pass)
{
	this->pass = pass;
	return *this;
}

Camera_New& Camera_New::SetTarget(a<Target> target)
{
	this->target = target;
	return *this;
}

void Camera_New::Render()
{
	Render(target);
}

void Camera_New::Render(r<Target> target)
{
	r<Target> t = target;
	pass->Run(lens, t);
}
