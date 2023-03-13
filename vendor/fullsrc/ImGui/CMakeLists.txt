cmake_minimum_required(VERSION 3.12)

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
  add_compile_options(/bigobj)
else ()
  #add_compile_options(-Wa,-mbig-obj)
endif ()

#
# Project
#

project(ImGui)

#
# Source files
#

file(GLOB_RECURSE sources CONFIGURE_DEPENDS "src/*" "backends/*")

#
# Output a static library
#

add_library(ImGui STATIC ${sources})

#
# Include everything in src, and from vendors
#

target_include_directories(ImGui
	PUBLIC include

	PRIVATE include/imgui
	PRIVATE src

	PRIVATE ../../public/include
	PRIVATE ../SDL2/include
	PRIVATE
)