#pragma once

enum TextureFormat
{
	// Legal to cast a channel count to the first 4

	// 1 byte
	TextureFormatR = 1,

	// 2 bytes
	TextureFormatRG,

	// 3 bytes
	TextureFormatRGB,

	// 4 bytes
	TextureFormatRGBA,

	// 1 32 bit int
	TextureFormatInt32,

	// 1 32 bit float
	TextureFormatFloat32,

	TextureFormatDepth,
	TextureFormatStencil
};

enum TextureFilter
{
	TextureFilterPixel,
	TextureFilterSmooth
};

//	Defines the layout of data in a texture's buffer
//
struct TextureLayout
{
	int width = 0;
	int height = 1;
	int depth = 1;
	TextureFormat format = TextureFormatRGBA;
	//TextureFilter filter = TextureFilterPixel;

	int Index(int x, int y) const;
	int Index(int x, int y, int z) const;

	bool InBounds(int index) const;

	int NumberOfBytesPerPixel() const;
	int NumberOfBytes() const;
	int NumberOfDimensions() const;

	bool Equals(const TextureLayout& other) const;

	bool operator==(const TextureLayout& other) const;
	bool operator!=(const TextureLayout& other) const;
};