-- Uncomment this if you want to build this standalone

--workspace "Winter"
--	configurations { "Debug", "Release" }
--	platforms { "x64" }

group "vendor"
	include "vendor/box2d"
	include "vendor/hitbox"
	include "vendor/imgui"
group ""

include "premake5.vars.lua"

project "Framework"
	kind "StaticLib"
	language "C++"
	cppdialect "C++17"
	location "lib/build"
	targetdir "lib/%{cfgdir}"

	files (src_all_vendor)
	includedirs (include_all_vendor)
	defines (all_defines)

	files { 
		"src/**.h",
		"src/impl/**.cpp",

		-- choose which extensions you need
		-- todo: move some of these to game, last 3 ?

		"src/ext/impl/physics/ClosestPointOnShape.cpp",
		"src/ext/impl/rendering/BatchLineRenderer.cpp",
		"src/ext/impl/rendering/BatchSpriteRenderer.cpp",
		"src/ext/impl/rendering/SimpleRender.cpp",
		"src/ext/impl/rendering/DebugRender.cpp",
		"src/ext/impl/serial/*.cpp",
		"src/ext/impl/AssetStore.cpp"
	}

	includedirs {
		"src"
	}

	if asset_root_path ~= nil then
		defines {
			"ASSET_ROOT_PATH=\"" .. asset_root_path .. "/\""
		}
	end

	filter "system:Windows"
		libdirs {
			libdir_sdl_win32
		}

		includedirs {
			include_sdl_win32
		}

		defines { "IW_PLATFORM_WINDOWS" }

	filter "system:macosx"
		libdirs {
			"/opt/homebrew/Cellar/sdl2/2.0.22/lib"
		}

		includedirs {
			"/opt/homebrew/Cellar/sdl2/2.0.22/include"
		}

	filter "configurations:Debug"
		defines { "IW_DEBUG", "DEBUG" }
		symbols "On"

	filter "configurations:Release"
		defines { "NDEBUG" }
		optimize "On"