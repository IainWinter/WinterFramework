workspace "Winter Framework"
	configurations { "Debug", "Release" }
	platforms { "x64" }

group "vendor"
	include "vendor/box2d"
	include "vendor/hitbox"
	include "vendor/imgui"
group ""

include_all_vendor = {
	"vendor/box2d/include",
	"vendor/imgui/include",
	"vendor/hitbox/include",
	"vendor/entt/include",
	"vendor/stb/include",
	"vendor/glm/include",
	"vendor/glad/include",
	"vendor/asio/include",
	"vendor/json/include",
	"vendor/fmod/include",
	"vendor/tinyglft/include",
	"vendor/sqlite/include",
}

src_all_vendor = {
	"vendor/glad/src/glad.c",
	"vendor/imgui/backends/imgui_impl_sdl.cpp",
	"vendor/imgui/backends/imgui_impl_opengl3.cpp",
	"vendor/stb/src/load_image.cpp",
	"vendor/sqlite/src/sqlite3.c"
}

libdir_all_vendor = {
	"vendor/box2d/lib",
	"vendor/imgui/lib",
	"vendor/hitbox/lib",
	"vendor/fmod/lib",
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

project "Framework"
	kind "StaticLib"
	language "C++"
	cppdialect "C++17"
	location "_build"
	targetdir "_bin"

	files (src_all_vendor)
	includedirs (include_all_vendor)
	defines (all_defines)

	files { 
		"framework/**.h",
		"framework/impl/**.cpp",

		-- choose which extensions you need

		"framework/ext/impl/Time.cpp",
		"framework/ext/impl/physics/ClosestPointOnShape.cpp"
	}

	includedirs {
		"framework"	
	}

	filter "system:Windows"
		includedirs {
			"vendor/sdl/include"
		}

		libdirs {
			"vendor/sdl/lib"
		}

		defines { "IW_PLATFORM_WINDOWS" }

	filter "configurations:Debug"
		defines { "IW_DEBUG", "DEBUG" }
		symbols "On"

	filter "configurations:Release"
		defines { "NDEBUG" }
		optimize "On"

project "Game"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++17"
	location "_build"
	targetdir "_bin"

	includedirs (include_all_vendor)
	libdirs (libdir_all_vendor)
	links (links_all_vendor)
	defines (all_defines)

	files { 
		--"editor/**.h",
		--"editor/**.cpp",

		"game/**.h",
		"game/**.cpp"
	}

	includedirs {
		"framework",
		"game"
	}

	libdirs {
		"_bin"
	}

	links {
		"framework"
	}

	filter "system:Windows"
		includedirs {
			"vendor/sdl/include"
		}

		libdirs {
			"vendor/sdl/lib"
		}

		links (links_windows)

		defines { "IW_PLATFORM_WINDOWS" }

	filter "configurations:Debug"
		defines { "IW_DEBUG", "DEBUG" }
		symbols "On"

	filter "configurations:Release"
		defines { "NDEBUG" }
		optimize "On"
