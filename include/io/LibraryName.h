#pragma once

#include <string>

// Take the name of a library and pre/append to it based on the operating system
std::string GetLibraryName(const std::string& name);
