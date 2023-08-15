#pragma once

#include "v2/RenderSystemOptions.h"

using GLenum = unsigned int;
using GLuint = unsigned int;
using GLint = int;

GLenum gl_format(v2TextureFormat format);
GLenum gl_iformat(v2TextureFormat format);
GLenum gl_type(v2TextureFormat format);
int    gl_num_channels(v2TextureFormat format);
int    gl_bytes_per_channel(v2TextureFormat format);
GLenum gl_filter(v2TextureFilter filter);
GLenum gl_tex_target(int dimensionCount);