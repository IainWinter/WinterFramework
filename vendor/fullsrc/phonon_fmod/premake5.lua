project "phonon_fmod"
	kind "SharedLib"
	language "C++"
	cppdialect "C++17"
	location "lib/build"
	targetdir "lib/%{cfgdir}"

	files {
		"../_shared/include/phonon/**.h",
		"../_shared/include/phonon_fmod/**.h",
		"src/**.h",
		"src/**.cpp",
	}

	includedirs {
		"include/phonon_fmod/",		
		"../_shared/include/phonon/",
		"../_shared/include"
	}
	
	filter "system:Windows"
		defines {
			"IPL_OS_WINDOWS",
			"NOMINMAX"
		}
	
	filter "system:macosx"
		defines {
			"IPL_OS_MACOSX",
			"NOMINMAX"
		}

	filter "configurations:Debug"
		symbols "On"

	filter "configurations:Release"
		optimize "On"
