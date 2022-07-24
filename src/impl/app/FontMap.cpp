#include "app/FontMap.h"

namespace FontMap
{
	std::unordered_map<std::string, ImFont*> Fonts;

	void Load(const std::string& name, float size, const std::string& path)
	{
		ImFontAtlas* atlas = ImGui::GetIO().Fonts;
		ImFont* font = atlas->AddFontFromFileTTF(_a(path).c_str(), size);
		Fonts.emplace(name, font);
	}

	ImFont* Get(const std::string& name)
	{
		return Fonts.at(name);
	}
}