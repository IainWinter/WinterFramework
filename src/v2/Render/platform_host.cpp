#include "v2/Render/platform_host.h"
#include <cstring>

TextureView _texture_host_alloc(const TextureLayout& layout)
{
	size_t size = (size_t)layout.NumberOfBytes();
	u8* buffer = (u8*)malloc(size);

	// don't complain about uninitialized memory
	if (!buffer)
		throw nullptr;

	memset(buffer, 0, size);

	return TextureView(buffer, layout);
}

void _texture_host_free(TextureView* view)
{
	if (!view->buffer)
		return;

	free(view->buffer);
	*view = {};
}

void _texture_host_copy(TextureView* into, const TextureView& other)
{
	if (into->layout != other.layout) // could just compare number of bytes
		return;
	
	const TextureLayout& layout = into->layout;
	
	size_t size = (size_t)layout.NumberOfBytes();
	memcpy(into->buffer, other.buffer, size);
}

void _texture_host_realloc(TextureView* view, const TextureLayout& newLayout)
{
	TextureLayout oldLayout = view->layout;
	
	// If the texture can just be reinterpreted, just change layout
	if (oldLayout.NumberOfBytes() == newLayout.NumberOfBytes())
	{
		*view = TextureView(view->buffer, newLayout);
	}
	
	// or recreate memory
	else
	{
		_texture_host_free(view);
		*view = _texture_host_alloc(newLayout);
	}
}
