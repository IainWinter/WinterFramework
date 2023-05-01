#pragma once

#include "Rendering.h"
#include "v2/Render/CameraLens.h"

//	Defines the sequence of steps to render a scene
//
class RenderPass
{
public:
	void Run(const CameraLens& lens, r<Target> target);
	void Init();

protected:
	//	Run the render pass (must be called on render thread)
	//
	virtual void _Run(const CameraLens& lens, r<Target> target) = 0;

	//	On first run of the render pass, Init values
	//
	virtual void _Init() {}

public:
	std::unordered_map<std::string, float> inputs;
	std::string name;
private:
	bool hasInit = false;
};