#include "v2/Render/RenderTranslateOptions.h"
#include "util/typedef.h"
#include "glad/glad.h"

GLenum gl_format(TextureFormat format)
{
	switch (format)			
	{
		case TextureFormatR:       return GL_RED;
		case TextureFormatRG:      return GL_RG;
		case TextureFormatRGB:     return GL_RGB;
		case TextureFormatRGBA:    return GL_RGBA;
		case TextureFormatDepth:   return GL_DEPTH_COMPONENT;
		case TextureFormatStencil: return GL_STENCIL_INDEX;
		case TextureFormatInt32:   return GL_RGBA_INTEGER;
		case TextureFormatFloat32: return GL_RGBA; // this doesnt have to specify float???
	}

	return -1;
}

GLenum gl_iformat(TextureFormat format)
{
	switch (format)
	{
		case TextureFormatR:       return GL_R8;
		case TextureFormatRG:      return GL_RG8;
		case TextureFormatRGB:     return GL_RGB8;
		case TextureFormatRGBA:    return GL_RGBA8;
		case TextureFormatDepth:   return GL_DEPTH_COMPONENT32;
		case TextureFormatStencil: return GL_STENCIL_INDEX8;
		case TextureFormatInt32:   return GL_RGBA32I;
		case TextureFormatFloat32: return GL_RGBA32F;
	}

	return -1;
}

GLenum gl_type(TextureFormat format)
{
	switch (format)
	{
		case TextureFormatR:       return GL_UNSIGNED_BYTE;
		case TextureFormatRG:      return GL_UNSIGNED_BYTE;
		case TextureFormatRGB:     return GL_UNSIGNED_BYTE;
		case TextureFormatRGBA:    return GL_UNSIGNED_BYTE;
		case TextureFormatDepth:   return GL_FLOAT;
		case TextureFormatStencil: return GL_UNSIGNED_BYTE;
		case TextureFormatInt32:   return GL_INT;
		case TextureFormatFloat32: return GL_FLOAT;
	}

	return -1;
}

int gl_num_channels(TextureFormat format)
{
	switch (format)
	{
		case TextureFormatR:        return 1;
		case TextureFormatRG:       return 2;
		case TextureFormatRGB:      return 3;
		case TextureFormatRGBA:     return 4;
		case TextureFormatDepth:    return 1;
		case TextureFormatStencil:  return 1;
		case TextureFormatInt32:    return 4;
		case TextureFormatFloat32:  return 4;
	}

	return -1;
}

int gl_bytes_per_channel(TextureFormat format)
{
	switch (format)
	{
		case TextureFormatR:        return sizeof(u8);
		case TextureFormatRG:       return sizeof(u8);
		case TextureFormatRGB:      return sizeof(u8);
		case TextureFormatRGBA:     return sizeof(u8);
		case TextureFormatDepth:    return sizeof(f32);
		case TextureFormatStencil:  return sizeof(u8);
		case TextureFormatInt32:    return sizeof(u32);
		case TextureFormatFloat32:  return sizeof(f32);
	}

	return -1;
}

GLenum gl_filter(TextureFilter filter)
{
	switch (filter)
	{
		case TextureFilterSmooth: return GL_LINEAR;
		case TextureFilterPixel:  return GL_NEAREST;
	}

	return -1;
}

GLenum gl_tex_target(int dimensionCount)
{
	switch (dimensionCount)
	{
		case 1: return GL_TEXTURE_1D;
		case 2: return GL_TEXTURE_2D;
		case 3: return GL_TEXTURE_3D;
	}

	return -1;
}