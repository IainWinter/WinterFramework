#include "v2/Render/platform_gl.h"
#include "util/error_check.h" // gives gl

TextureHandle _texture_device_alloc(const TextureViewConst& view)
{
	log_render("_texture_device_alloc %p ", view.GetBytes());

	u32 handle = -1;
	u32 target = gl_tex_target(view.GetLayout().NumberOfDimensions());
	
	gl(glGenTextures(1, &handle));
	
	TextureHandle texture = TextureHandle(handle, target, view.GetLayout());
	
	_texture_device_bind(texture);
	_texture_device_filter_set(texture, fPixel);
	_texture_device_realloc(&texture, view);
	
	return texture;
}

void _texture_device_free(TextureHandle* texture)
{
	if (!texture->HasData())
		return;

	log_render("_texture_device_free %d ", texture->GetHandle());

	GLuint handle = texture->GetHandle();
	
	gl(glDeleteTextures(1, &handle));
	*texture = {};
}

void _texture_device_bind(const TextureHandle& texture)
{
	log_render("_texture_device_bind %d -> %d", texture.GetTarget(), texture.GetHandle());

	gl(glBindTexture(texture.GetTarget(), texture.GetHandle()));
}

void _texture_device_realloc(TextureHandle* texture, const TextureViewConst& view)
{
	log_render("_texture_device_realloc %d -> %d", texture->GetTarget(), texture->GetHandle());

	const TextureLayout& layout = view.GetLayout();
	const u8* buffer = view.GetBytes();
	
	GLenum iformat = gl_iformat(layout.format);
	GLenum format = gl_format(layout.format);
	GLenum type = gl_type(layout.format);
	GLint width = layout.width;
	GLint height = layout.height;
	GLint depth = layout.depth;

	GLuint target = texture->GetTarget();
	
	switch (layout.NumberOfDimensions())
	{
		case 1:
			gl(glTexImage1D(target, 0, iformat, width, 0, format, type, buffer));
			break;
		case 2:
			gl(glTexImage2D(target, 0, iformat, width, height, 0, format, type, buffer));
			break;
		case 3:
			gl(glTexImage3D(target, 0, iformat, width, height, depth, 0, format, type, buffer));
			break;
	}
}

void _texture_device_copy_to_host(const TextureHandle& texture, const TextureView& view)
{
	if (texture.GetLayout() != view.GetLayout()) // If layouts are not the same, exit
	{
		// could bring in platform_host to realloc view, but I like that they are seperate
		// just resize host before calling this function
		return;
	}
	
	TextureFormat format = texture.GetLayout().format;
	
	GLenum target = texture.GetTarget();
	GLenum gformat = gl_format(format);
	GLenum type = gl_type(format);

	gl(glBindTexture(target, texture.GetHandle()));
	gl(glGetTexImage(target, 0, gformat, type, view.GetBytes()));
}

void _texture_device_filter_set(const TextureHandle& texture, TextureFilter filter)
{
	GLuint target = texture.GetTarget();
	GLenum gfilter = gl_filter(filter);
	
	gl(glTexParameteri(target, GL_TEXTURE_MIN_FILTER, gfilter));
	gl(glTexParameteri(target, GL_TEXTURE_MAG_FILTER, gfilter));
}

void _texture_device_clamp_set(const TextureHandle& texture /*, TextureClamp clamp */)
{
	GLuint target = texture.GetTarget();
	GLenum clampW = GL_CLAMP_TO_EDGE;
	GLenum clampH = GL_CLAMP_TO_EDGE;
	GLenum clampD = GL_CLAMP_TO_EDGE;
	
	int dimCount = texture.GetLayout().NumberOfDimensions();
	
	if (dimCount >= 1) gl(glTexParameteri(target, GL_TEXTURE_WRAP_S, clampW));
	if (dimCount >= 2) gl(glTexParameteri(target, GL_TEXTURE_WRAP_T, clampH));
	if (dimCount >= 3) gl(glTexParameteri(target, GL_TEXTURE_WRAP_R, clampD));
}