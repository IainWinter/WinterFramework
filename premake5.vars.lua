cfgdir = "%{cfg.buildcfg}"

include_all_vendor = {
	path.getabsolute("vendor/box2d/include"),
	path.getabsolute("vendor/imgui/include"),
	path.getabsolute("vendor/hitbox/include"),
	path.getabsolute("vendor/entt/include"),
	path.getabsolute("vendor/stb/include"),
	path.getabsolute("vendor/glm/include"),
	path.getabsolute("vendor/glad/include"),
	path.getabsolute("vendor/asio/include"),
	path.getabsolute("vendor/json/include"),
	path.getabsolute("vendor/fmod/include"),
	path.getabsolute("vendor/tinyglft/include"),
	path.getabsolute("vendor/sqlite/include"),
	path.getabsolute("vendor/cr/include"),
	path.getabsolute("vendor/tinyfiledialogs/include")
}

src_all_vendor = {
	path.getabsolute("vendor/glad/src/glad.c"),
	path.getabsolute("vendor/imgui/backends/imgui_impl_sdl.cpp"),
	path.getabsolute("vendor/imgui/backends/imgui_impl_opengl3.cpp"),
	path.getabsolute("vendor/stb/src/load_image.cpp"),
	path.getabsolute("vendor/sqlite/src/sqlite3.c"),
	path.getabsolute("vendor/tinyfiledialogs/src/tinyfiledialogs.c")
}

libdir_all_vendor = {
	path.getabsolute("vendor/box2d/lib/%{cfgdir}"),
	path.getabsolute("vendor/imgui/lib/%{cfgdir}"),
	path.getabsolute("vendor/hitbox/lib/%{cfgdir}"),
	path.getabsolute("vendor/fmod/lib")
}

libdir_sdl_win32 = {
	path.getabsolute("vendor/sdl/lib")
}

include_sdl_win32 = {
	path.getabsolute("vendor/sdl/include")
}

src_framework = {
	path.getabsolute("src")
}

lib_framework = {
	path.getabsolute("lib/%{cfgdir}")
}

links_all_vendor = {
	"SDL2",
	"box2d",
	"ImGui", 
	"hitbox",
	"fmod",
	"fmodstudio"
}

links_windows = {
	"gdi32",
	"ws2_32",
	"wsock32",
	"user32",
	"kernel32",
	"shell32"
}

all_defines = {
	"SDL_MAIN_HANDLED",
	"IW_NOLOOP"
}