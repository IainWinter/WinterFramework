#pragma once

#include <tuple>
#include <string>
#include "Log.h"

using u8 = unsigned char;

std::tuple<u8*, int, int, int> load_image(const std::string& filepath);
void free_image(void* pixels);