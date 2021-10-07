cmake_minimum_required(VERSION 3.16.4)

project(flame)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

add_definitions(-W0 -std:c++latest)

include("utils.cmake")

foreach(type ${CMAKE_CONFIGURATION_TYPES})
	string(TOUPPER ${type} type_u)
	string(TOLOWER ${type} type_l)
	set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${type_u} "${CMAKE_SOURCE_DIR}/bin/${type_l}")
	set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${type_u} "${CMAKE_SOURCE_DIR}/bin/${type_l}")
	set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${type_u} "${CMAKE_SOURCE_DIR}/bin/${type_l}")
endforeach()

string(FIND "${CMAKE_LINKER}" "/VC/" pos)
string(SUBSTRING  "${CMAKE_LINKER}" 0 ${pos} VS_LOCATION)

get_filename_component(parent_path "${CMAKE_SOURCE_DIR}" DIRECTORY)

check_path("${parent_path}glm" p)
set(GLM_INCLUDE_DIR "${p}" CACHE PATH "GLM include directory")
if (NOT EXISTS "${GLM_INCLUDE_DIR}")
	message(SEND_ERROR "required GLM is invalid")
endif()

check_path("${parent_path}pugixml/src" p)
set(PUGIXML_INCLUDE_DIR "${p}" CACHE PATH "Pugi XML directory")
check_path("${parent_path}pugixml/build/debug/pugixml.lib" p)
set(PUGIXML_DEBUG_LIB "${p}" CACHE FILEPATH "Pugi XML debug LIB Path")
check_path("${parent_path}pugixml/build/release/pugixml.lib" p)
set(PUGIXML_RELEASE_LIB "${p}" CACHE FILEPATH "Pugi XML release LIB Path")
if (NOT EXISTS "${PUGIXML_INCLUDE_DIR}" OR NOT EXISTS "${PUGIXML_DEBUG_LIB}" OR NOT EXISTS "${PUGIXML_RELEASE_LIB}")
	message(SEND_ERROR "required Pugi XML is invalid")
endif()

check_path("${parent_path}njson/include" p)
set(NJSON_INCLUDE_DIR "${p}" CACHE PATH "NJson include directory")
if (NOT EXISTS "${NJSON_INCLUDE_DIR}")
	message(SEND_ERROR "required NJson is invalid")
endif()

check_path("${parent_path}cppcodec" p)
set(CPPCODEC_INCLUDE_DIR "${p}" CACHE PATH "cppcodec include directory")
if (NOT EXISTS "${NJSON_INCLUDE_DIR}")
	message(SEND_ERROR "required cppcodec is invalid")
endif()

check_path("${parent_path}boost/regex/include" p)
set(BOOST_REGEX_INCLUDE_DIR "${p}" CACHE PATH "Boost Regex include directory")
if (NOT EXISTS "${BOOST_REGEX_INCLUDE_DIR}")
	message(SEND_ERROR "required Boost Regex is invalid")
endif()

check_path("${parent_path}stb" p)
set(STB_INCLUDE_DIR "${p}" CACHE PATH "STB include directory")
if (NOT EXISTS "${STB_INCLUDE_DIR}")
	message(SEND_ERROR "required STB is invalid")
endif()

check_path("${parent_path}sha1" p)
set(SHA1_INCLUDE_DIR "${p}" CACHE PATH "SHA1 include directory")
if (NOT EXISTS "${SHA1_INCLUDE_DIR}")
	message(STATUS "optional SHA1 is invalid, turn off USE_SHA1")
	set(USE_SHA1 off)
else()
	set(USE_SHA1 on)
endif()

check_path("$ENV{ProgramFiles} (x86)/OpenAL 1.1 SDK/include" p)
set(OPENAL_INCLUDE_DIR "${p}" CACHE PATH "OpenAL include directory")
check_path("$ENV{ProgramFiles} (x86)/OpenAL 1.1 SDK/libs/Win64/OpenAL32.lib" p)
set(OPENAL_LIB_PATH "${p}" CACHE FILEPATH "OpenAL LIB Path")
if (NOT EXISTS "${OPENAL_INCLUDE_DIR}" OR NOT EXISTS "${OPENAL_LIB_PATH}")
	message(SEND_ERROR "required OpenAL is invalid")
endif()

check_path("${parent_path}lua/inc" p)
set(LUA_INCLUDE_DIR "${p}" CACHE PATH "LUA include directory")
check_path("${parent_path}lua/build/debug/lua.lib" p)
set(LUA_LIB_PATH "${p}" CACHE FILEPATH "LUA LIB Path")
check_path("${parent_path}lua/build/debug/lua.dll" p)
set(LUA_DLL_PATH "${p}" CACHE FILEPATH "LUA DLL Path")
if (NOT EXISTS "${LUA_INCLUDE_DIR}" OR NOT EXISTS "${LUA_LIB_PATH}" OR NOT EXISTS "${LUA_DLL_PATH}")
	message(SEND_ERROR "required LUA is invalid")
endif()

set(MYSQL_INCLUDE_DIR "" CACHE PATH "MySQL include directory")
set(MYSQL_LIB_PATH "" CACHE FILEPATH "MySQL LIB Path")
set(MYSQL_DLL_PATH "" CACHE FILEPATH "MySQL DLL Path")

set(SPIRV_CROSS_INCLUDE_DIR "" CACHE PATH "SPIRV Cross include directory")
set(SPIRV_CROSS_DEBUG_LIBS_DIR "" CACHE PATH "SPIRV Cross debug LIBs directory")
set(SPIRV_CROSS_RELEASE_LIBS_DIR "" CACHE PATH "SPIRV Cross release LIBs directory")

set(GLI_INCLUDE_DIR "" CACHE PATH "GLI include directory")

set(ASSIMP_INCLUDE_DIR "" CACHE PATH "Assimp include directory")
set(ASSIMP_LIB_PATH "" CACHE FILEPATH "Assimp LIB Path")
set(ASSIMP_DLL_PATH "" CACHE FILEPATH "Assimp DLL Path")

set(PHYSX_INCLUDE_DIR "" CACHE PATH "PhysX directory")
set(PHYSX_DEBUG_LIBS_DIR "" CACHE PATH "PhysX debug LIBs directory")
set(PHYSX_RELEASE_LIBS_DIR "" CACHE PATH "PhysX release LIBs directory")

set(ASSETS_DIR "${CMAKE_SOURCE_DIR}/assets")
set(SHADERS_DIR "${ASSETS_DIR}/shaders")

add_subdirectory(source)
add_subdirectory(tests)
add_subdirectory(tools)
add_subdirectory(games)
