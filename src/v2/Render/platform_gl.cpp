#include "v2/Render/platform_gl.h"
#include "util/error_check.h" // gives gl

TextureHandle _texture_device_alloc(const TextureView& view)
{
	log_render("_texture_device_alloc %p ", view.buffer);

	u32 handle = -1;
	u32 target = gl_tex_target(view.layout.NumberOfDimensions());
	
	gl(glGenTextures(1, &handle));
	
	TextureHandle texture = TextureHandle(handle, target, view.layout);
	
	_texture_device_bind(texture);
	_texture_device_filter_set(texture, TextureFilterPixel);
	_texture_device_realloc(&texture, view);
	
	return texture;
}

void _texture_device_free(TextureHandle* texture)
{
	if (!texture->handle)
		return;

	log_render("_texture_device_free %d ", texture->handle);

	GLuint handle = texture->handle;
	
	gl(glDeleteTextures(1, &handle));
	*texture = {};
}

void _texture_device_bind(const TextureHandle& texture)
{
	log_render("_texture_device_bind %d -> %d", texture.type, texture.handle);

	gl(glBindTexture(texture.type, texture.handle));
}

void _texture_device_realloc(TextureHandle* texture, const TextureView& view)
{
	log_render("_texture_device_realloc %d -> %d", texture->type, texture->handle);

	const TextureLayout& layout = view.layout;
	const u8* buffer = view.buffer;
	
	GLenum iformat = gl_iformat(layout.format);
	GLenum format = gl_format(layout.format);
	GLenum type = gl_type(layout.format);
	GLint width = layout.width;
	GLint height = layout.height;
	GLint depth = layout.depth;

	GLuint target = texture->type;
	
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
	if (texture.layout != view.layout) // If layouts are not the same, exit
	{
		// could bring in platform_host to realloc view, but I like that they are separate
		// just resize host before calling this function
		return;
	}
	
	TextureFormat format = texture.layout.format;
	
	GLenum target = texture.type;
	GLenum gformat = gl_format(format);
	GLenum type = gl_type(format);

	gl(glBindTexture(target, texture.handle));
	gl(glGetTexImage(target, 0, gformat, type, view.buffer));
}

void _texture_device_filter_set(const TextureHandle& texture, TextureFilter filter)
{
	GLuint target = texture.type;
	GLenum gfilter = gl_filter(filter);
	
	gl(glTexParameteri(target, GL_TEXTURE_MIN_FILTER, gfilter));
	gl(glTexParameteri(target, GL_TEXTURE_MAG_FILTER, gfilter));
}

void _texture_device_clamp_set(const TextureHandle& texture /*, TextureClamp clamp */)
{
	GLuint target = texture.type;
	GLenum clampW = GL_CLAMP_TO_EDGE;
	GLenum clampH = GL_CLAMP_TO_EDGE;
	GLenum clampD = GL_CLAMP_TO_EDGE;
	
	int dimCount = texture.layout.NumberOfDimensions();
	
	if (dimCount >= 1) gl(glTexParameteri(target, GL_TEXTURE_WRAP_S, clampW));
	if (dimCount >= 2) gl(glTexParameteri(target, GL_TEXTURE_WRAP_T, clampH));
	if (dimCount >= 3) gl(glTexParameteri(target, GL_TEXTURE_WRAP_R, clampD));
}