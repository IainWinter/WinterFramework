#pragma once

#include "v2/Render/RenderTranslateOptions.h"
#include "v2/Render/TextureView.h"
#include "util/typedef.h"

// Create a new texture on the device and allocate a buffer and copy data in from the specified view
TextureHandle _texture_device_alloc(const TextureViewConst& view);

// Destroy a texture and its buffer on the device
void _texture_device_free(TextureHandle* texture);

// Bind a texture for use, call this before any of the _texture_device functions
void _texture_device_bind(const TextureHandle& texture);

// Create a copy of the texture on the device, need to impl, only works on Gl 4.0 + without doing a render pass
// so maybe just leave to the user, I've never needed to copy a texture, so wont bother right now
//void _texture_device_copy(TextureHandle texture);

// Reallocate the buffer of a texture on the device according to specified view
void _texture_device_realloc(TextureHandle* texture, const TextureViewConst& view);

// Read the textures buffer into the specified view. View must have the same layout as the texture
void _texture_device_copy_to_host(const TextureHandle& texture, const TextureView& view);

void _texture_device_filter_set(const TextureHandle& texture, TextureFilter filter);
void _texture_device_clamp_set(const TextureHandle& texture /*, TextureClamp clamp ... */);