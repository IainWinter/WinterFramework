#pragma once

#include "v2/Render/TextureLayout.h"
#include "util/Color.h"

//	Defines a buffer in host RAM with a defined layout from TextureLayout
//  Does not own memory
//
struct TextureView
{
	TextureView() 
		: buffer (nullptr)
		, layout ()
	{}

	TextureView(const TextureLayout& layout)
		: buffer (nullptr)
		, layout (layout)
	{}
	
	TextureView(u8* buffer, const TextureLayout& layout)
		: buffer (buffer)
		, layout (layout)
	{}

	// Return a Color from an index. Layout defines which accessors in Color are valid
	// For example, an fRG texture will have Color::r and Color::g valid, but Color::b and Color::a will be the next pixel's data
	Color& at(int index1D) {
		return *(Color*)(buffer + index1D * layout.NumberOfBytesPerPixel());
	}

	u8* buffer;
	TextureLayout layout;
};

//  Holds a reference to a device handle of a texture
//  Does not own handle
//
class TextureHandle
{
public:
	TextureHandle()
		: handle (0)
		, type   (0)
	{}
	
	TextureHandle(u32 handle, u32 type, const TextureLayout& layout)
		: handle (handle)
		, type   (type)
		, layout (layout)
	{}

	u32 handle;
	u32 type;
	TextureLayout layout;
};
