#include "v2/Render/RenderPass.h"

void RenderPass::Run(const CameraLens& lens, r<Target> target)
{
	if (!hasInit)
		Init();

	_Run(lens, target);
}

void RenderPass::Init()
{
	_Init();
	hasInit = true;
}