#pragma once

#include "util/Transform3D.h"
#include "v2/Render/RenderPass.h"
#include "ext/AssetItem.h"

//	Defines a camera with a view & projection matrix. This also hold a reference to
//	a target and render pass for drawing.
//
class Camera_New
{
public:
	CameraLens lens;

	a<Target> target;
	a<RenderPass> pass;

public:
	Camera_New& SetLens(const CameraLens& lens);

	Camera_New& SetPass(a<RenderPass> pass);
	Camera_New& SetTarget(a<Target> target);

	// Render this camera now
	void Render();

	// Render this camera now for a specific target
	void Render(r<Target> target);
};