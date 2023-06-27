#include "v2/Render/TextureLayout.h"

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
		case TextureFormatR:       return 1;
		case TextureFormatRG:      return 2;
		case TextureFormatRGB:     return 3;
		case TextureFormatRGBA:    return 4;
		case TextureFormatInt32:   return 4;
		case TextureFormatFloat32: return 4;
		case TextureFormatDepth:   return 4;
		case TextureFormatStencil: return 1;
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
	if (height >= 1) return 2;
	return 1;
}

bool TextureLayout::Equals(const TextureLayout& other) const
{
	return width == other.width
		&& height == other.height
		&& depth == other.depth
		&& format == other.format;
}

bool TextureLayout::operator==(const TextureLayout& other) const
{
	return Equals(other);
}

bool TextureLayout::operator!=(const TextureLayout& other) const
{
	return !Equals(other);
}