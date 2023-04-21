cmake_minimum_required(VERSION 3.12)

#
# Include libs that need to be built
#

set(ASSIMP_NO_EXPORT ON)
set(ASSIMP_BUILD_TESTS OFF)
set(ASSIMP_INSTALL_PDB OFF)
set(ASSIMP_INSTALL OFF)
set(LIBRARY_SUFFIX "")
set(ASSIMP_INJECT_DEBUG_POSTFIX OFF)
set(ASSIMP_BUILD_ALL_EXPORTERS_BY_DEFAULT OFF)
set(ASSIMP_BUILD_GLTF_IMPORTER TRUE)
add_subdirectory(vendor/fullsrc/assimp vendor/assimp)

set(SDL2_DISABLE_INSTALL ON)
set(SDL2_DISABLE_SDL2MAIN ON)
set(SDL2_DISABLE_UNINSTALL ON)
set(SDL_AUDIO OFF)
add_subdirectory(vendor/fullsrc/SDL2 vendor/SDL2)

add_subdirectory(vendor/fullsrc/box2d vendor/box2d)
add_subdirectory(vendor/fullsrc/ImGui vendor/ImGui)
add_subdirectory(vendor/fullsrc/phonon_fmod vendor/phonon_fmod)

set(TBB_TEST OFF)
add_subdirectory(vendor/fullsrc/tbb vendor/tbb)

#
# Set C++ version and output dirs
#

set(CMAKE_CXX_STANDARD 20)
# set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/lib/${CMAKE_BUILD_TYPE})
# set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/lib/${CMAKE_BUILD_TYPE})

#
# Set build file settings
#

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

if (CMAKE_GENERATOR MATCHES "Visual Studio")
  add_compile_options(/bigobj /MP)
else ()
  #add_compile_options(-Wa,-mbig-obj)
endif ()

if (WIN32)
	set(VENDOR_PREBUILT_DIR vendor/prebuilt/win_x64)
elseif (APPLE)
	set(VENDOR_PREBUILT_DIR vendor/prebuilt/mac_arm64)
elseif (LINUX)	
	set(VENDOR_PREBUILT_DIR vendor/prebuilt/linux_x64)
endif ()

#
# Project
#

project(WinterFramework)

#
# Source files
#

file(GLOB_RECURSE sources CONFIGURE_DEPENDS "include/*" "src/*" "vendor/public/src/*" "vendor/private/src/*")

#
# Output a static library
#

add_library(WinterFramework STATIC ${sources})

#
# Link vendor libraries
#

target_link_directories(WinterFramework PRIVATE ${VENDOR_PREBUILT_DIR})

target_link_libraries(WinterFramework
	PRIVATE assimp
	PRIVATE box2d
	PRIVATE SDL2
	PRIVATE ImGui
	PRIVATE phonon_fmod
	PRIVATE tbb
	PRIVATE fmod
	PRIVATE fmodstudio
	PRIVATE phonon
)

#
# Include everything in src, and from vendors
#

target_include_directories(WinterFramework
	PUBLIC include
	PRIVATE src

	# should be PRIVATE
	PUBLIC vendor/private/include

	PUBLIC vendor/public/include

	PRIVATE vendor/fullsrc/assimp/include
	PUBLIC vendor/fullsrc/box2d/include
	PUBLIC vendor/fullsrc/SDL2/include
	PUBLIC vendor/fullsrc/ImGui/include
	PUBLIC vendor/fullsrc/phonon_fmod/include
	PUBLIC vendor/fullsrc/tbb/include
)

#
# Definitions
#

target_compile_definitions(WinterFramework 
	PRIVATE ASSET_ROOT_PATH="./assets"
	PUBLIC SDL_MAIN_HANDLED
)
