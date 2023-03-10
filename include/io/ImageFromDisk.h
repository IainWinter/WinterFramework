#pragma once

struct RawImageData
{
	char* buffer;
	int width, height, channels;
};

// Free the image data with free()
RawImageData io_LoadImageFromFile(const char* filepath);