cmake_minimum_required(VERSION 3.4)

project(flame)

option(FLAME_BUILD_SHARED_LIBRARY "FLAME_BUILD_SHARED_LIBRARY" OFF)
option(FLAME_ENABLE_PHYSICS "FLAME_ENABLE_PHYSICS" OFF)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

add_definitions(-W0)
add_definitions(-DFLAME_WINDOWS)

include(cmake/utils.cmake)

set_output_dir("${CMAKE_SOURCE_DIR}/bin")

if(CMAKE_SIZEOF_VOID_P MATCHES "8")
	set(FLAME_SYS_NAME "x64")
	set(FLAME_SYS_BIT "64")
else()
	set(FLAME_SYS_NAME "x86")
	set(FLAME_SYS_BIT "32")
endif()

set(RAPIDXML_PATH "" CACHE PATH "RapidXML Path")
set(STB_PATH "" CACHE PATH "STB Path")
set(FREETYPE_PATH "" CACHE PATH "FreeType Path")
set(SPRIV_CROSS_PATH "" CACHE PATH "SPIRV-Cross Path")
if (FLAME_ENABLE_MODEL)
set(ASSIMP_PATH "" CACHE PATH "Assimp Path")
endif()
set(OPENAL_PATH "" CACHE PATH "OpenAL Path")

#configure_file(src/flame/config.in ${CMAKE_SOURCE_DIR}/src/flame/config.h)

add_subdirectory(source)
add_subdirectory(tests)
add_subdirectory(games)
add_subdirectory(tools)
