#pragma once

#include "Defines.h"
#include "imgui/imgui.h"
#include <unordered_map>
#include <string>

// doesnt unload anything...

namespace FontMap
{
	void Load(const std::string& name, float size, const std::string& path);
	ImFont* Get(const std::string& name);
}