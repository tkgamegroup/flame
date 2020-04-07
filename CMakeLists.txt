cmake_minimum_required(VERSION 3.16.4)

project(flame)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

add_definitions(-W0 -std:c++latest)

include("cmake_utils.cmake")

foreach(type ${CMAKE_CONFIGURATION_TYPES})
	string(TOUPPER ${type} type_u)
	string(TOLOWER ${type} type_l)
	set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${type_u} "${CMAKE_SOURCE_DIR}/bin/${type_l}")
	set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${type_u} "${CMAKE_SOURCE_DIR}/bin/${type_l}")
	set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${type_u} "${CMAKE_SOURCE_DIR}/bin/${type_l}")
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
set(GRAPHVIZ_PATH "" CACHE PATH "Graphviz Path (optional)")
set(PUGIXML_INCLUDE_DIR "" CACHE PATH "Pugi XML Directory")
set(PUGIXML_DEBUG_STATIC_LIBRARY_PATH "" CACHE FILEPATH "Pugi XML Debug STATIC Library Path")
set(PUGIXML_RELEASE_STATIC_LIBRARY_PATH "" CACHE FILEPATH "Pugi XML Release STATIC Library Path")
set(NJSON_INCLUDE_DIR "" CACHE PATH "NJson Include Directory")
set(STB_INCLUDE_DIR "" CACHE PATH "STB Include Directory")
set(OPENAL_INCLUDE_DIR "" CACHE PATH "OpenAL Include Directory")
set(OPENAL_STATIC_LIBRARY_PATH "" CACHE FILEPATH "OpenAL STATIC Library Path")

option(FLAME_ENABLE_MODEL "Enable Model" OFF)
if (FLAME_ENABLE_MODEL)
	set(ASSIMP_INCLUDE_DIR "" CACHE PATH "Assimp Include Directory")
	set(ASSIMP_STATIC_LIBRARY_PATH "" CACHE FILEPATH "Assimp STATIC Library Path")
	set(ASSIMP_SHARED_LIBRARY_PATH "" CACHE FILEPATH "Assimp SHARED Library Path")
endif()

option(FLAME_ENABLE_PHYSICS "Enable Physics" OFF)
if (FLAME_ENABLE_PHYSICS)
	set(PHYSX_DIR "" CACHE PATH "Physx Directory")
	set(PHYSX_STATIC_LIBRARY_DIR "" CACHE PATH "PHYSX STATIC Library Directory")
	set(PHYSX_SHARED_LIBRARY_DIR "" CACHE PATH "PHYSX SHARED Library Directory")
endif()

set(INCLUDE_DIR "${CMAKE_SOURCE_DIR}/include/flame")

add_subdirectory(source)
add_subdirectory(tests)
add_subdirectory(tools)
add_subdirectory(games)
