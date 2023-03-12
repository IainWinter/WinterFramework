project "ImGui"
	kind "StaticLib"
	language "C++"
	cppdialect "C++17"
	location "lib/build"
	targetdir "lib/%{cfgdir}"

	files {
        	"src/*.cpp"
	}

	includedirs {
		"include/imgui",
		"src",

		"../_shared/include", -- this is for glm conversion to ImVec2
		"../_shared/include/_win/SDL2" -- imgui expects SDL2 to be not in a folder
    	}

	filter "configurations:Debug"
		defines { "DEBUG" }
		symbols "On"

	filter "configurations:Release"
		defines { "NDEBUG" }
		optimize "On"

