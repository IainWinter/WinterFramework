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

#
# Project
#

project(Hitbox)

#
# Source files
#

file(GLOB_RECURSE sources CONFIGURE_DEPENDS "./src/*")

#
# Output
#

add_library(Hitbox STATIC ${sources})
