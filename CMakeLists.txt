cmake_minimum_required(VERSION 3.4)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

add_definitions(-W0)

foreach(c ${CMAKE_CONFIGURATION_TYPES})
	string(TOUPPER ${c} c )
	set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${c} "${CMAKE_SOURCE_DIR}/bin")
	set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${c} "${CMAKE_SOURCE_DIR}/bin")
	set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${c} "${CMAKE_SOURCE_DIR}/bin")
endforeach()

project(flame)

function(add_static_lib name source_list folder)
	add_library(${name} STATIC ${source_list})
	set_target_properties(${name} PROPERTIES FOLDER ${folder})
endfunction()

function(add_lib name source_list folder)
	add_library(${name} SHARED ${source_list})
	set_target_properties(${name} PROPERTIES FOLDER ${folder})
endfunction()

function(add_exe name source_list folder)
	add_executable(${name} ${source_list})
	set_target_properties(${name} PROPERTIES FOLDER ${folder})
	set_target_properties(${name} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")
endfunction()

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
set(RAPIDXML_PATH "" CACHE PATH "RapidXML Path")
set(STB_PATH "" CACHE PATH "STB Path")
set(FREETYPE_PATH "" CACHE PATH "FreeType Path")
set(SPRIV_CROSS_PATH "" CACHE PATH "SPIRV-Cross Path")

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
