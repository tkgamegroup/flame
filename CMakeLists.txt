cmake_minimum_required(VERSION 3.4)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

add_definitions(-W0)
add_definitions(-DFLAME_WINDOWS)

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

set(PLATFORM "Windows" CACHE PATH "Platform") # or Android

option(FLAME_ENABLE_PHYSICS "FLAME_ENABLE_PHYSICS" OFF)

set(VS_PATH "" CACHE PATH "VisualStudio Path")
set(RAPIDXML_PATH "" CACHE PATH "RapidXML Path")
set(STB_PATH "" CACHE PATH "STB Path")
set(FREETYPE_PATH "" CACHE PATH "FreeType Path")
set(SPRIV_CROSS_PATH "" CACHE PATH "SPIRV-Cross Path")
if (FLAME_ENABLE_MODEL)
set(ASSIMP_PATH "" CACHE PATH "Assimp Path")
endif()
set(OPENAL_PATH "" CACHE PATH "OpenAL Path")

add_subdirectory(source)
add_subdirectory(test)
add_subdirectory(game)
add_subdirectory(tool)
