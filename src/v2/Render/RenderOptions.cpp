#include "v2/Render/RenderOptions.h"

int TextureLayout::Index(int x, int y) const
{
	return x + y * width;
}

int TextureLayout::Index(int x, int y, int z) const
{
	return x + y * width + z * width * height;
}

bool TextureLayout::InBounds(int index) const
{
	return index >= 0 && index < width * height * depth;
}

int TextureLayout::NumberOfBytesPerPixel() const
{
	switch (format)
	{
		case fR:       return 1;
		case fRG:      return 2;
		case fRGB:     return 3;
		case fRGBA:    return 4;
		case fInt32:   return 4;
		case fFloat32: return 4;
		case fDepth:   return 4;
		case fStencil: return 1;
	}

	return -1;
}

int TextureLayout::NumberOfBytes() const
{
	return width * height * depth * NumberOfBytesPerPixel();
}

int TextureLayout::NumberOfDimensions() const
{
	if (depth > 1) return 3;
	if (height > 1) return 2;
	return 1;
}

bool TextureLayout::operator==(const TextureLayout& other) const
{
	return Equals(other);
}

bool TextureLayout::operator!=(const TextureLayout& other) const
{
	return !Equals(other);
}

bool TextureLayout::Equals(const TextureLayout& other) const
{
	return width == other.width
		&& height == other.height
		&& depth == other.depth
		&& format == other.format;
}