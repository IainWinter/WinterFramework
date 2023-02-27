-- Uncomment this if you want to build this standalone

--workspace "Winter"
--	configurations { "Debug", "Release" }
--	platforms { "x64" }

group "vendor"
	include "vendor/box2d"
	include "vendor/hitbox"
	include "vendor/imgui"
	include "vendor/phonon_fmod"
group ""

include "premake5.vars.lua"

project "WinterFramework"
	kind "StaticLib"
	language "C++"
	cppdialect "C++20"
	location "lib/build"
	targetdir "lib/%{cfgdir}"

	flags {
		"MultiProcessorCompile" 
	}
	
	files (src_vendor)
	includedirs (include_vendor)
	defines (defines_all)
	libdirs (libdir_vendor_self)

	files { 
		"src/**.h",
		"src/impl/**.cpp",

		-- choose extensions, right now it's all of them

		"src/ext/impl/**.h",
		"src/ext/impl/**.cpp"
	}

	includedirs {
		"src"
	}

	filter "system:Windows"
		--libdirs (libdir_vendor_win_x64)
		includedirs (include_vendor_win)
		defines (defines_win)
		buildoptions ("/bigobj /FS")

	filter "system:macosx"
		libdirs (libdir_vendor_mac_arm64)
		includedirs (include_vendor_mac)
		defines (defines_mac)

	filter "system:linux"
		defines (defines_linux)
		
	filter "configurations:Debug"
		defines { "IW_DEBUG", "DEBUG" }
		symbols "On"

	filter "configurations:Release"
		defines { "NDEBUG" }
		optimize "On"