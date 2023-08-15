#pragma once

// Legal to cast a channel count to the first 4
enum v2TextureFormat
{
	v2TextureFormatR = 1,
	v2TextureFormatRG,
	v2TextureFormatRGB,
	v2TextureFormatRGBA,

	v2TextureFormatInt32,
	v2TextureFormatFloat32,

	v2TextureFormatDepth,
	v2TextureFormatStencil
};

enum v2TextureFilter
{
	v2TextureFilterPixel,
	v2TextureFilterSmooth
};