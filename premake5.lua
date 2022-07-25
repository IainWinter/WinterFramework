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
		"src/impl/**.cpp",

		"src/ext/impl/Time.cpp",
		"src/ext/impl/physics/ClosestPointOnShape.cpp",

		"vendor/glad/src/glad.c",
		"vendor/imgui/backends/imgui_impl_sdl.cpp",
		"vendor/imgui/backends/imgui_impl_opengl3.cpp",
		"vendor/stb/src/load_image.cpp",
		--"vendor/sqlite/src/sqlite3.c",

		-- these are game files, remove for just framework

		--"test/Window_Test.cpp",
		"game/**.h",
		"game/**.cpp"
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
		"vendor/json/include",
		"vendor/fmod/include",

		--"vendor/sqlite/include",

		-- these are game files, see above

		"game"
	}

	libdirs {
		"vendor/box2d/lib",
		"vendor/imgui/lib",
		"vendor/hitbox/lib",
		"vendor/fmod/lib",
	}

	links {
		"SDL2",
		"box2d",
		"ImGui", 
		"hitbox",
		"fmod",
		"fmodstudio"
	}

	defines { 
		"IW_DEBUG",
		"SDL_MAIN_HANDLED"
	}

	buildoptions {
		"/bigobj" -- for issue with C1128, is this a sign something is incorrect? maybe too many headers I havent been using cpp files too much for simplicity
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

		defines { "IW_PLATFORM_WINDOWS" }

	filter "configurations:Debug"
		defines { "DEBUG" }
		symbols "On"

	filter "configurations:Release"
		defines { "NDEBUG" }
		optimize "On"

