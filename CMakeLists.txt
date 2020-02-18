cmake_minimum_required(VERSION 3.16.4)

project(flame)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

add_definitions(-W0 -std:c++latest)

include("cmake_utils.cmake")

foreach(c ${CMAKE_CONFIGURATION_TYPES})
	string(TOUPPER ${c} c )
	set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${c} "${CMAKE_SOURCE_DIR}/bin")
	set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${c} "${CMAKE_SOURCE_DIR}/bin")
	set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${c} "${CMAKE_SOURCE_DIR}/bin")
endforeach()

set(FLAME_PLATFORM "Windows" CACHE STRING "Platform")
set_property(CACHE FLAME_PLATFORM PROPERTY STRINGS Windows Android)
if (FLAME_PLATFORM MATCHES "Windows")
	add_definitions(-DFLAME_WINDOWS)
elseif (FLAME_PLATFORM MATCHES "Android")
	add_definitions(-DFLAME_ANDROID)
endif()

set(FLAME_RENDER_API "Vulkan" CACHE STRING "Render API")
set_property(CACHE FLAME_RENDER_API PROPERTY STRINGS Vulkan D3D12)
if (FLAME_RENDER_API MATCHES "Vulkan")
	add_definitions(-DFLAME_VULKAN)
elseif (FLAME_RENDER_API MATCHES "D3D12")
	add_definitions(-DFLAME_D3D12)
endif()

if (FLAME_PLATFORM MATCHES "Android" AND FLAME_RENDER_API MATCHES "D3D12")
	message(FATAL_ERROR "cannot use D3D on Android")
endif()

set(VS_PATH "C:\\Program Files (x86)\\Microsoft Visual Studio\\2019\\Community" CACHE PATH "VisualStudio Path")
set(PUGIXML_PATH "" CACHE PATH "Pugi XML Path")
set(NJSON_PATH "" CACHE PATH "NJson Path")
set(STB_PATH "" CACHE PATH "STB Path")
set(FREETYPE_PATH "" CACHE PATH "FreeType Path")
set(MSDFGEN_PATH "" CACHE PATH "Msdfgen Path")
set(SPRIV_CROSS_PATH "" CACHE PATH "SPIRV-Cross Path")
set(GRAPHVIZ_PATH "" CACHE PATH "Graphviz Path (optional)")

option(FLAME_ENABLE_MODEL "Enable Model" OFF)
if (FLAME_ENABLE_MODEL)
	set(ASSIMP_PATH "" CACHE PATH "Assimp Path")
endif()

option(FLAME_ENABLE_PHYSICS "Enable Physics" OFF)
if (FLAME_ENABLE_PHYSICS)
	set(PHYSX_PATH "" CACHE PATH "Physx Path")
endif()

set(OPENAL_PATH "" CACHE PATH "OpenAL Path")

add_subdirectory(source)
add_subdirectory(test)
add_subdirectory(game)
add_subdirectory(tool)
