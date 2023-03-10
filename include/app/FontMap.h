#pragma once

#include <string>

// doesn't unload anything...

struct ImFont;

namespace FontMap
{
	void Load(const std::string& name, float size, const std::string& path);
	ImFont* Get(const std::string& name);
}
