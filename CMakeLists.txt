cmake_minimum_required(VERSION 3.16.4)

project(flame)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

enable_language(ASM_MASM)

add_definitions(-W0 -std:c++latest)

include("cmake_utils.cmake")

foreach(type ${CMAKE_CONFIGURATION_TYPES})
	string(TOUPPER ${type} type_u)
	string(TOLOWER ${type} type_l)
	set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${type_u} "${CMAKE_SOURCE_DIR}/bin/${type_l}")
	set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${type_u} "${CMAKE_SOURCE_DIR}/bin/${type_l}")
	set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${type_u} "${CMAKE_SOURCE_DIR}/bin/${type_l}")
endforeach()

string(FIND "${CMAKE_LINKER}" "/VC/" pos)
string(SUBSTRING  "${CMAKE_LINKER}" 0 ${pos} VS_LOCATION)

get_filename_component(dir ${CMAKE_SOURCE_DIR} DIRECTORY)

set(GRAPHVIZ_PATH "" CACHE PATH "Graphviz Path (optional)")

if (EXISTS "${dir}/pugixml")
	set(temp "${dir}/pugixml")
else()
	set(temp "")
endif()
set(PUGIXML_INCLUDE_DIR "${temp}/src" CACHE PATH "Pugi XML Directory")
set(PUGIXML_DEBUG_STATIC_LIBRARY_PATH "${temp}/build/Debug/pugixml.lib" CACHE FILEPATH "Pugi XML Debug STATIC Library Path")
set(PUGIXML_RELEASE_STATIC_LIBRARY_PATH "${temp}/build/Release/pugixml.lib" CACHE FILEPATH "Pugi XML Release STATIC Library Path")

if (EXISTS "${dir}/json")
	set(temp "${dir}/json")
else()
	set(temp "")
endif()
set(NJSON_INCLUDE_DIR "${temp}/include" CACHE PATH "NJson Include Directory")

if (EXISTS "${dir}/stb")
	set(temp "${dir}/stb")
else()
	set(temp "")
endif()
set(STB_INCLUDE_DIR "${temp}" CACHE PATH "STB Include Directory")

if (EXISTS "${dir}/OpenAL 1.1 SDK")
	set(temp "${dir}/OpenAL 1.1 SDK")
else()
	set(temp "")
endif()
set(OPENAL_INCLUDE_DIR "${temp}/include" CACHE PATH "OpenAL Include Directory")
set(OPENAL_STATIC_LIBRARY_PATH "${temp}/libs/Win64/OpenAL32.lib" CACHE FILEPATH "OpenAL STATIC Library Path")

#[[
set(ASSIMP_INCLUDE_DIR "" CACHE PATH "Assimp Include Directory")
set(ASSIMP_STATIC_LIBRARY_PATH "" CACHE FILEPATH "Assimp STATIC Library Path")
set(ASSIMP_SHARED_LIBRARY_PATH "" CACHE FILEPATH "Assimp SHARED Library Path")

set(PHYSX_DIR "" CACHE PATH "Physx Directory")
set(PHYSX_STATIC_LIBRARY_DIR "" CACHE PATH "PHYSX STATIC Library Directory")
set(PHYSX_SHARED_LIBRARY_DIR "" CACHE PATH "PHYSX SHARED Library Directory")
]]

set(INCLUDE_DIR "${CMAKE_SOURCE_DIR}/include/flame")

add_subdirectory(source)
add_subdirectory(tests)
add_subdirectory(tools)
add_subdirectory(games)
