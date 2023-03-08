cmake_minimum_required(VERSION 3.12)

#
# Set C++ version and output dirs
#

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/lib/${CMAKE_BUILD_TYPE})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/lib/${CMAKE_BUILD_TYPE})

#
# Project
#

project(WinterFramework)

#
# Source files
#

file(GLOB_RECURSE sources CONFIGURE_DEPENDS "./src/*.cpp")

#
# Include everything in src, and from vendors
#

include_directories(./src)
include_directories(./vendor/_shared/include)
include_directories(./vendor/box2d/include)
include_directories(./vendor/fmod/include)
include_directories(./vendor/hitbox/include)
include_directories(./vendor/imgui/include)
include_directories(./vendor/phonon_fmod/include)

include_directories(/opt/homebrew/include)

#
# Definitions
#
add_compile_definitions(ASSET_ROOT_PATH)

#
# Output
#

add_library(WinterFramework STATIC ${sources})
