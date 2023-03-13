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

project(phonon_fmod)

#
# Source files
#

file(GLOB_RECURSE sources CONFIGURE_DEPENDS "src/*")

#
# Output a static library
#

add_library(phonon_fmod SHARED ${sources})

#
# Include everything in src, and from vendors
#

target_include_directories(phonon_fmod
	PUBLIC include

	PRIVATE include/phonon_fmod
	PRIVATE ../../private/include
	PRIVATE ../../private/include/phonon
)

if (WIN32)
	target_compile_definitions(phonon_fmod 
		PUBLIC IPL_OS_WINDOWS
		PUBLIC NOMINMAX
		PRIVATE UNICODE 
	)
elseif (APPLE)
	target_compile_definitions(phonon_fmod
		PUBLIC IPL_OS_MACOSX
	)
endif ()