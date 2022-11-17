#pragma once

#include "Common.h"

#include <string>
#include <unordered_map>
#include <sstream>

// generate a random string of characters of a specified length
std::string nonce(int length = 16, const char* prefix = "##");

// map a nonce to an instance
const char* nonce_cached(void* instance, int length = 16, const char* prefix = "##");