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

		-- choose which extensions you need, right now it's all of them

		"src/ext/impl/**.h",
		"src/ext/impl/**.cpp"
	}

	includedirs {
		"src"
	}

	defines {
		"wTOOLS=\"" .. path.getabsolute("tools/") .. "\""
	}

	filter "system:Windows"
		libdirs {
			libdir_sdl_win32
		}

		includedirs {
			include_sdl_win32
		}

		defines {
			"wPREMAKE=\"premake5.exe\"",
			"wPREMAKE_BUILD=\"vs2022\"",
			"wBUILD_CLI=\"msbuild\""
		}

		buildoptions ("/bigobj /FS")

	filter "system:macosx"
		libdirs {
			"/opt/homebrew/Cellar/sdl2/2.0.22/lib"
		}

		includedirs {
			"/opt/homebrew/Cellar/sdl2/2.0.22/include"
		}

		defines {
			"wPREMAKE=\"mac_premake5\"",
			"wPREMAKE_BUILD=\"xcode4\"",
			"wBUILD_CLI=\"needs link\""
		}

	filter "system:linux"
		defines {
			"wPREMAKE=\"premake5\"",
			"wPREMAKE_BUILD=\"gmake2\"",
			"wBUILD_CLI=\"make\""
		}

	filter "configurations:Debug"
		defines { "IW_DEBUG", "DEBUG" }
		symbols "On"

	filter "configurations:Release"
		defines { "NDEBUG" }
		optimize "On"