cmake_minimum_required(VERSION 3.12)

#
# Set C++ version and output dirs
#

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/lib/${CMAKE_BUILD_TYPE})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/lib/${CMAKE_BUILD_TYPE})

#
# Set build file settings
#

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

if (CMAKE_GENERATOR MATCHES "Visual Studio")
  add_compile_options(/bigobj)
else ()
  add_compile_options(-Wa,-mbig-obj)
endif ()

#
# Project
#

project(WinterFramework)

#
# Source files
#

file(GLOB_RECURSE sources CONFIGURE_DEPENDS "src/*")

#
# Include everything in src, and from vendors
#

include_directories(src)
include_directories(vendor/_shared/include)
include_directories(vendor/box2d/include)
include_directories(vendor/fmod/include)
include_directories(vendor/hitbox/include)
include_directories(vendor/imgui/include)
include_directories(vendor/phonon_fmod/include)

if (WIN32)
	include_directories(vendor/_shared/include/_win)
else ()
	include_directories(/opt/homebrew/include)
endif ()

#
# Definitions
#

add_compile_definitions(ASSET_ROOT_PATH)

if (WIN32)
	add_compile_definitions(IPL_OS_WINDOWS)
elseif (APPLE)
	add_compile_definitions(IPL_OS_MACOSX)
endif ()

#
# Output
#

add_library(WinterFramework STATIC ${sources})

#
# Link vendor
#

find_library(HitboxTarget Hitbox PATHS vendor/Hitbox/lib)

target_link_libraries(WinterFramework HitboxTarget)
