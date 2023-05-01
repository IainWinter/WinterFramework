#include "v2/Render/RenderTranslateOptions.h"
#include "util/typedef.h"
#include "glad/glad.h"

GLenum gl_format(TextureFormat format)
{
	switch (format)			
	{
		case TextureFormat::fR:       return GL_RED;
		case TextureFormat::fRG:      return GL_RG;
		case TextureFormat::fRGB:     return GL_RGB;
		case TextureFormat::fRGBA:    return GL_RGBA;
		case TextureFormat::fDepth:   return GL_DEPTH_COMPONENT;
		case TextureFormat::fStencil: return GL_STENCIL_INDEX;
		case TextureFormat::fInt32:   return GL_RGBA_INTEGER;
		case TextureFormat::fFloat32: return GL_RGBA; // this doesnt have to specify float???
	}

	return -1;
}

GLenum gl_iformat(TextureFormat format)
{
	switch (format)
	{
		case TextureFormat::fR:       return GL_R8;
		case TextureFormat::fRG:      return GL_RG8;
		case TextureFormat::fRGB:     return GL_RGB8;
		case TextureFormat::fRGBA:    return GL_RGBA8;
		case TextureFormat::fDepth:   return GL_DEPTH_COMPONENT32;
		case TextureFormat::fStencil: return GL_STENCIL_INDEX8;
		case TextureFormat::fInt32:   return GL_RGBA32I;
		case TextureFormat::fFloat32: return GL_RGBA32F;
	}

	return -1;
}

GLenum gl_type(TextureFormat format)
{
	switch (format)
	{
		case TextureFormat::fR:       return GL_UNSIGNED_BYTE;
		case TextureFormat::fRG:      return GL_UNSIGNED_BYTE;
		case TextureFormat::fRGB:     return GL_UNSIGNED_BYTE;
		case TextureFormat::fRGBA:    return GL_UNSIGNED_BYTE;
		case TextureFormat::fDepth:   return GL_FLOAT;
		case TextureFormat::fStencil: return GL_UNSIGNED_BYTE;
		case TextureFormat::fInt32:   return GL_INT;
		case TextureFormat::fFloat32: return GL_FLOAT;
	}

	return -1;
}

int gl_num_channels(TextureFormat format)
{
	switch (format)
	{
		case TextureFormat::fR:        return 1;
		case TextureFormat::fRG:       return 2;
		case TextureFormat::fRGB:      return 3;
		case TextureFormat::fRGBA:     return 4;
		case TextureFormat::fDepth:    return 1;
		case TextureFormat::fStencil:  return 1;
		case TextureFormat::fInt32:    return 4;
		case TextureFormat::fFloat32:  return 4;
	}

	return -1;
}

int gl_bytes_per_channel(TextureFormat format)
{
	switch (format)
	{
		case TextureFormat::fR:        return sizeof(u8);
		case TextureFormat::fRG:       return sizeof(u8);
		case TextureFormat::fRGB:      return sizeof(u8);
		case TextureFormat::fRGBA:     return sizeof(u8);
		case TextureFormat::fDepth:    return sizeof(f32);
		case TextureFormat::fStencil:  return sizeof(u8);
		case TextureFormat::fInt32:    return sizeof(u32);
		case TextureFormat::fFloat32:  return sizeof(f32);
	}

	return -1;
}

GLenum gl_filter(TextureFilter filter)
{
	switch (filter)
	{
		case TextureFilter::fSmooth: return GL_LINEAR;
		case TextureFilter::fPixel:  return GL_NEAREST;
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