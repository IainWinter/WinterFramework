#pragma once

#include "v2/Render/RenderOptions.h"

// fwd
using GLenum = unsigned int;
using GLuint = unsigned int;
using GLint = int;

GLenum gl_format              (TextureFormat format);
GLenum gl_iformat             (TextureFormat format);
GLenum gl_type                (TextureFormat format);
int    gl_num_channels        (TextureFormat format);
int    gl_bytes_per_channel   (TextureFormat format);
GLenum gl_filter              (TextureFilter filter);
GLenum gl_tex_target          (int dimensionCount);

//GLenum gl_attachment          (Target::AttachmentName name);
//int    gl_element_type_size   (Buffer::ElementType type);
//GLenum gl_buffer_draw         (bool isStatic);
//GLenum gl_format              (Buffer::ElementType type);
//GLenum gl_drawtype            (Mesh::Topology drawType);
//GLenum gl_shader_type         (ShaderProgram::ShaderName type);
//GLint  gl_program_texture_slot(int slot);