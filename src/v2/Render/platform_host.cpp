#include "v2/Render/platform_host.h"
#include <cstring>

TextureView _texture_host_alloc(const TextureLayout& layout)
{
	size_t size = (size_t)layout.NumberOfBytes();
	u8* buffer = (u8*)malloc(size);

	if (!buffer)
		throw nullptr;

	// don't complain about uninitialized memory
	memset(buffer, 0, size);

	return TextureView(buffer, layout);
}

void _texture_host_free(TextureView* view)
{
	if (!view->HasData())
		return;

	free(view->GetBytes());
	*view = {};
}

void _texture_host_copy(TextureView* into, const TextureView& other)
{
	if (into->GetLayout() != other.GetLayout()) // could just compare number of bytes
	{
		return;
	}
	
	const TextureLayout& layout = into->GetLayout();
	
	size_t size = (size_t)layout.NumberOfBytes();
	memcpy(into->GetBytes(), other.GetBytes(), size);
}

void _texture_host_realloc(TextureView* view, const TextureLayout& newLayout)
{
	TextureLayout oldLayout = view->GetLayout();
	
	// If the texture can just be reinterpreted, just change layout
	if (oldLayout.NumberOfBytes() == newLayout.NumberOfBytes())
	{
		*view = TextureView(view->GetBytes(), newLayout);
	}
	
	// or recreate memory
	else
	{
		_texture_host_free(view);
		*view = _texture_host_alloc(newLayout);
	}
}
