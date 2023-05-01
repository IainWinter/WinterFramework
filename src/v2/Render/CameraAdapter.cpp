#include "v2/Render/CameraAdapter.h"

Camera FromCameraLens(const CameraLens& lens)
{
	Camera c;
	c.position = lens.position;
	c.height = lens.height;
	c.width = lens.height * lens.aspect;
	c.is_ortho = lens.ortho;
	c.near = lens.near;
	c.far = lens.far;
	c.rotation = lens.rotation;

	return c;
}
