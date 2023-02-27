#include "stb/load_image.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

std::tuple<u8*, int, int, int> load_image(const std::string& filepath)
{
	int width, height, channels, format;
	stbi_info(filepath.c_str(), &width, &height, &channels);

	stbi_set_flip_vertically_on_load(true);

	switch (channels)
	{
		case 1: format = STBI_grey;       break;
		case 2: format = STBI_grey_alpha; break;
		case 3: format = STBI_rgb;        break;
		case 4: format = STBI_rgb_alpha;  break;
	}

	u8* pixels = stbi_load(filepath.c_str(), &width, &height, &channels, format);

	if (!pixels /*|| stbi_failure_reason()*/) // no SOI bug
	{
		log_io("failed to load image '%s' reason: %s", filepath.c_str(), stbi_failure_reason());
	}

	return std::make_tuple(pixels, width, height, channels);
}

void free_image(void* pixels)
{
	free(pixels); // stb calls free, doesnt HAVE to though so this is a lil jank
}