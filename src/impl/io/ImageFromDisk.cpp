#include "io/ImageFromDisk.h"

#include "Log.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

RawImageData io_LoadImageFromFile(const char* filepath)
{
    int width, height, channels, format;
	stbi_info(filepath, &width, &height, &channels);

	stbi_set_flip_vertically_on_load(true);

	switch (channels)
	{
		case 1: format = STBI_grey;       break;
		case 2: format = STBI_grey_alpha; break;
		case 3: format = STBI_rgb;        break;
		case 4: format = STBI_rgb_alpha;  break;
		default:
			log_io("w~Failed to load image '%s' reason: Invalid number of channels", filepath);
			return {};
	}

	char* pixels = (char*)stbi_load(filepath, &width, &height, &channels, format);

	if (!pixels /*|| stbi_failure_reason()*/) // no SOI bug
	{
		log_io("w~Failed to load image '%s' reason: %s", filepath, stbi_failure_reason());
	}

	return RawImageData{ pixels, width, height, channels };
}
