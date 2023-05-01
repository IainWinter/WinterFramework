#pragma once

enum TextureFormat
{
	// Legal to cast a channel count to the first 4

	// 1 byte
	fR = 1,

	// 2 bytes
	fRG,

	// 3 bytes
	fRGB,

	// 4 bytes
	fRGBA,

	// 1 32 bit int
	fInt32,

	// 1 32 bit float
	fFloat32,

	fDepth,
	fStencil
};

enum TextureFilter
{
	fPixel,
	fSmooth
};

enum TextureAccess
{
	aHost,
	aDevice,
	aHostDevice
};

//	Defines the layout of data in a texture's buffer
//
struct TextureLayout
{
	int width;
	int height = 1;
	int depth = 1;
	TextureFormat format = fRGB;

	int Index(int x, int y) const;
	int Index(int x, int y, int z) const;

	bool InBounds(int index) const;

	int NumberOfBytesPerPixel() const;
	int NumberOfBytes() const;
	int NumberOfDimensions() const;

	bool operator==(const TextureLayout& other) const;
	bool operator!=(const TextureLayout& other) const;

	bool Equals(const TextureLayout& other) const;
};

//	
//
enum TargetName
{
	aColor,
	aColor1,
	aColor2,
	aColor3,
	aColor4,

	aDepth
};