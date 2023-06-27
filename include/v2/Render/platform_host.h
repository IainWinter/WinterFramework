#pragma once

#include "v2/Render/TextureView.h"
#include "v2/Render/MeshView.h"
#include "util/typedef.h"

// Allocate a buffer based on the specified layout
TextureView _texture_host_alloc(const TextureLayout& layout);

// Free the memory in a view, and reset it
void _texture_host_free(TextureView* view);

// Copy the memory from a view into another, into must have the same layout as other
void _texture_host_copy(TextureView* into, const TextureView& other);

// Reformat the view to match the new layout, assume the memory is wiped
void _texture_host_realloc(TextureView* view, const TextureLayout& newLayout);