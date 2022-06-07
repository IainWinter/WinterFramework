workspace "Winter Framework"
	configurations { "Debug", "Release" }
	platforms { "x64" }

group "vendor"
	include "vendor/box2d"
	include "vendor/hitbox"
	include "vendor/imgui"
group ""

project "Framework"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++17"
	location "_build"
	targetdir "_bin"

	files { 
		"src/**.h",

		"src/ext/cpp/Time.cpp",

		"vendor/glad/src/glad.c",
		"vendor/imgui/backends/imgui_impl_sdl.cpp",
		"vendor/imgui/backends/imgui_impl_opengl3.cpp",

		"test/Window_Test.cpp",
		--"game/main.cpp"
	}

	includedirs {
		"src",

		"vendor/box2d/include",
		"vendor/imgui/include",
		"vendor/hitbox/include",
		"vendor/entt/include",
		"vendor/stb/include",
		"vendor/glm/include",
		"vendor/glad/include",
		"vendor/asio/include",
		"vendor/json/include"
	}

	libdirs {
		"vendor/box2d/lib",
		"vendor/imgui/lib",
		"vendor/hitbox/lib" 
	}

	links {
		"SDL2",
		"box2d",
		"ImGui", 
		"hitbox"
	}

	defines { 
		"IW_DEBUG"
	}

	filter "system:Windows"
		includedirs {
			"vendor/sdl/include"
		}

		libdirs {
			"vendor/sdl/lib"
		}

		links {
			"gdi32",
			"ws2_32",
			"wsock32",
			"user32",
			"kernel32",
			"shell32"
		}

	filter "configurations:Debug"
		defines { "DEBUG" }
		symbols "On"

	filter "configurations:Release"
		defines { "NDEBUG" }
		optimize "On"

