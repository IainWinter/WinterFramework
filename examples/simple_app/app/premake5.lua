project "App"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++20"
	location "bin/build"
	targetdir "bin"

	flags { "MultiProcessorCompile" }

	includedirs (include_all_vendor)
	libdirs (libdir_all_vendor)
	links (links_all_vendor)
	defines (all_defines)

	defines {
		"ASSET_ROOT_PATH=\"" .. asset_root_path .. "\""
	}

	files {
		"src/**.h",
		"src/**.cpp"
	}

	includedirs {
		src_framework,
		"src"
	}

	libdirs {
		lib_framework
	}

	links {
		"framework"
	}
	
	filter "system:Windows"
		libdirs {
			libdir_sdl_win32
		}

		includedirs {
			include_sdl_win32
		}

		links (links_windows)
		buildoptions ("/bigobj /FS")

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
