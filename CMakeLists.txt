cmake_minimum_required(VERSION 3.25.0)
project(flame LANGUAGES CXX CSharp)

include("utils.cmake")
include("$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
add_definitions(-W0 -std:c++latest)

set_output_dir("${CMAKE_SOURCE_DIR}/bin")

string(FIND "${CMAKE_LINKER}" "/VC/" pos)
string(SUBSTRING  "${CMAKE_LINKER}" 0 ${pos} VS_LOCATION)

get_filename_component(parent_dir "${CMAKE_SOURCE_DIR}" DIRECTORY)

find_package(glm CONFIG REQUIRED)
if(NOT glm_FOUND)
	message(SEND_ERROR "required GLM is invalid")
endif()

find_package(pugixml CONFIG REQUIRED)
if(NOT pugixml_FOUND)
	message(SEND_ERROR "required Pugi XML is invalid")
endif()

find_package(nlohmann_json CONFIG REQUIRED)
if(NOT nlohmann_json_FOUND)
	message(SEND_ERROR "required nlohmann_json is invalid")
endif()

find_path(CPPCODEC_INCLUDE_DIRS "cppcodec/base32_crockford.hpp")
if(NOT EXISTS "${CPPCODEC_INCLUDE_DIRS}")
	message(SEND_ERROR "required cppcodec is invalid")
endif()

find_package(Stb REQUIRED)
if(NOT Stb_FOUND)
	message(SEND_ERROR "required STB is invalid")
endif()

set(USE_EXPRTK on CACHE BOOL "Use Exprtk")
if(USE_EXPRTK)
	find_path(EXPRTK_INCLUDE_DIRS "exprtk.hpp")
	if(NOT EXISTS "${EXPRTK_INCLUDE_DIRS}")
		message(STATUS "optional exprtk is invalid, turn off USE_EXPRTK")
		set(USE_EXPRTK off)
	endif()
endif()

set(USE_SHA1 on CACHE BOOL "Use SHA1")
if(USE_SHA1)
	set(SHA1_INCLUDE_DIR "" CACHE PATH "SHA1 include directory")
	if(NOT EXISTS "${SHA1_INCLUDE_DIR}")
		message(STATUS "optional SHA1 is invalid, turn off USE_SHA1")
		set(USE_SHA1 off)
	endif()
endif()

set(USE_CURL on CACHE BOOL "Use Curl")
if(USE_CURL)
	find_package(CURL)
	if(NOT CURL_FOUND)
		message(STATUS "optional curl is invalid, turn off USE_CURL")
		set(USE_CURL off)
	endif()
endif()

set(USE_MSDFGEN on CACHE BOOL "Use Msdfgen")
if(USE_MSDFGEN)
	find_package(msdfgen CONFIG)
	if(NOT msdfgen_FOUND)
		message(STATUS "optional msdfgen is invalid, turn off USE_MSDFGEN")
		set(USE_MSDFGEN off)
	endif()
endif()

set(RENDERING_BACKEND "VULKAN" CACHE STRING "Choose a rendering backend")
set_property(CACHE RENDERING_BACKEND PROPERTY STRINGS VULKAN D3D12)

if(RENDERING_BACKEND STREQUAL "VULKAN")
	if(NOT EXISTS "$ENV{VK_SDK_PATH}")
		message(SEND_ERROR "VULKAN is invalid!")
	endif()
	set(USE_D3D12 off)
	set(USE_VULKAN on)
endif()
if(RENDERING_BACKEND STREQUAL "D3D12")
	set(USE_D3D12 on)
	set(USE_VULKAN off)
endif()

if(USE_D3D12)
	find_package(directxtk12 CONFIG REQUIRED)
	if(NOT directxtk12_FOUND)
		message(SEND_ERROR "directxtk12 is invalid!")
	endif()
endif()

find_package(gli CONFIG REQUIRED)
if(NOT gli_FOUND)
	message(SEND_ERROR "required GLI is invalid")
endif()

find_package(spirv_cross_core CONFIG REQUIRED)
find_package(spirv_cross_glsl CONFIG REQUIRED)
find_package(spirv_cross_hlsl CONFIG REQUIRED)
find_package(spirv_cross_reflect CONFIG REQUIRED)
find_package(spirv_cross_cpp CONFIG REQUIRED)
if(NOT spirv_cross_core_FOUND OR NOT spirv_cross_glsl_FOUND OR NOT spirv_cross_reflect_FOUND OR NOT spirv_cross_cpp_FOUND OR (RENDERING_BACKEND STREQUAL "D3D12" AND NOT spirv_cross_hlsl_FOUND))
	message(SEND_ERROR "required SPIRV-Cross is invalid")
endif()

set(USE_NVTT on CACHE BOOL "Use NVTT")
if(USE_NVTT)
	check_path("C:/Program Files/NVIDIA Corporation/NVIDIA Texture Tools/include" p)
	set(NVTT_INCLUDE_DIR "${p}" CACHE PATH "NVIDIA Texture Tools include directory")
	check_path("C:/Program Files/NVIDIA Corporation/NVIDIA Texture Tools/lib/x64-v142/nvtt30106.lib" p)
	set(NVTT_LIB_PATH "${p}" CACHE FILEPATH "NVIDIA Texture Tools LIB path")
	check_path("C:/Program Files/NVIDIA Corporation/NVIDIA Texture Tools/nvtt30106.dll" p)
	set(NVTT_DLL_PATH "${p}" CACHE FILEPATH "NVIDIA Texture Tools DLL path")
	check_path("C:/Program Files/NVIDIA Corporation/NVIDIA Texture Tools/cudart64_110.dll" p)
	set(NVTT_CUDA_DLL_PATH "${p}" CACHE FILEPATH "NVIDIA Texture Tools CUDA DLL path")
	if(NOT EXISTS "${NVTT_INCLUDE_DIR}" OR NOT EXISTS "${NVTT_LIB_PATH}" OR NOT EXISTS "${NVTT_DLL_PATH}" OR NOT EXISTS "${NVTT_CUDA_DLL_PATH}")
		message(STATUS "optional NVIDIA Texture Tools is invalid, turn off USE_NVTT")
		set(USE_NVTT off)
	endif()
endif()

set(USE_ASSIMP on CACHE BOOL "Use Assimp")
if(USE_ASSIMP)
	find_package(assimp CONFIG)
	if(NOT assimp_FOUND)
		message(STATUS "optional Assimp is invalid, turn off USE_ASSIMP")
		set(USE_ASSIMP off)
	endif()
endif()

set(USE_FBXSDK on CACHE BOOL "Use FBX SDK")
if(USE_FBXSDK)
	check_path("C:/Program Files/Autodesk/FBX/FBX SDK/2020.3.4/include" p)
	set(FBXSDK_INCLUDE_DIR "${p}" CACHE PATH "FBX SDK include directory")
	check_path("C:/Program Files/Autodesk/FBX/FBX SDK/2020.3.4/lib/vs2022/x64/debug/libfbxsdk.lib" p)
	set(FBXSDK_LIB_PATH "${p}" CACHE FILEPATH "FBX SDK LIB path")
	check_path("C:/Program Files/Autodesk/FBX/FBX SDK/2020.3.4/lib/vs2022/x64/debug/libfbxsdk.dll" p)
	set(FBXSDK_DLL_PATH "${p}" CACHE FILEPATH "FBX SDK DLL Path")
	if(NOT EXISTS "${FBXSDK_INCLUDE_DIR}" OR NOT EXISTS "${FBXSDK_LIB_PATH}" OR NOT EXISTS "${FBXSDK_DLL_PATH}")
		message(STATUS "optional FBX SDK is invalid, turn off USE_FBXSDK")
		set(USE_FBXSDK off)
	endif()
endif()

set(USE_PHYSX on CACHE BOOL "Use PhysX")
if(USE_PHYSX)
	set(PHYSX_INCLUDE_DIR "" CACHE PATH "PhysX include directory")
	set(PHYSX_DEBUG_LIBS_DIR "" CACHE PATH "PhysX debug LIBs directory")
	set(PHYSX_RELEASE_LIBS_DIR "" CACHE PATH "PhysX release LIBs directory")
	if(NOT EXISTS "${PHYSX_INCLUDE_DIR}" OR (NOT EXISTS "${PHYSX_DEBUG_LIBS_DIR}" AND NOT EXISTS "${PHYSX_RELEASE_LIBS_DIR}"))
		message(STATUS "optional PhysX is invalid, turn off USE_PHYSX")
		set(USE_PHYSX off)
	endif()
endif()

set(USE_BOX2D on CACHE BOOL "Use Box2d")
if(USE_BOX2D)
	find_package(box2d CONFIG)
	if(NOT box2d_FOUND)
		message(STATUS "optional box2d is invalid, turn off USE_BOX2D")
		set(USE_BOX2D off)
	endif()
endif()

set(USE_RECASTNAV on CACHE BOOL "Use Recast Navigation")
if(USE_RECASTNAV)
	find_package(recastnavigation CONFIG)
	if(NOT recastnavigation_FOUND)
		message(STATUS "optional Recastnavigation is invalid, turn off USE_RECASTNAV")
		set(USE_RECASTNAV off)
	endif()
endif()

set(USE_FONT_AWESOME on CACHE BOOL "Use FONT-AWESOME")
if(USE_FONT_AWESOME)
	set(FONT_AWESOME_DIR "" CACHE PATH "Font-Awesome directory")
	if(NOT EXISTS "${FONT_AWESOME_DIR}")
		message(STATUS "optional font-awesome is invalid, turn off USE_FONT_AWESOME")
		set(USE_FONT_AWESOME off)
	endif()
endif()

set(USE_IMGUI on CACHE BOOL "Use ImGui")
if(USE_IMGUI)
	set(IMGUI_DIR "" CACHE PATH "imgui(docking branch) directory")
	if(NOT EXISTS "${IMGUI_DIR}")
		message(STATUS "optional imgui is invalid, turn off USE_IMGUI")
		set(USE_IMGUI off)
	endif()
endif()

set(USE_IM_GUIZMO on CACHE BOOL "Use ImGuizmo")
if(USE_IM_GUIZMO)
	set(IM_GUIZMO_DIR "" CACHE PATH "ImGuizmo directory")
	if(NOT EXISTS "${IM_GUIZMO_DIR}")
		message(STATUS "optional ImGuizmo is invalid, turn off USE_IM_GUIZMO")
		set(USE_IM_GUIZMO off)
	endif()
endif()

set(USE_IMGUI_NODE_EDITOR on CACHE BOOL "Use ImGui Node Editor")
if(USE_IMGUI_NODE_EDITOR)
	set(IMGUI_NODE_EDITOR_DIR "" CACHE PATH "ImGui Node Editor directory")
	if(NOT EXISTS "${IMGUI_NODE_EDITOR_DIR}")
		message(STATUS "optional ImGui Node Editor is invalid, turn off USE_IMGUI_NODE_EDITOR")
		set(USE_IMGUI_NODE_EDITOR off)
	endif()
endif()

set(USE_FORTUNE_ALGORITHM on CACHE BOOL "Use FortuneAlgorithm")
if(USE_FORTUNE_ALGORITHM)
	set(FORTUNE_ALGORITHM_DIR "" CACHE PATH "FortuneAlgorithm directory")
	if(NOT EXISTS "${FORTUNE_ALGORITHM_DIR}")
		message(STATUS "optional FortuneAlgorithm is invalid, turn off USE_FORTUNE_ALGORITHM")
		set(USE_FORTUNE_ALGORITHM off)
	endif()
endif()

set(USE_OPENAL on CACHE BOOL "Use OpenAL")
if(USE_OPENAL)
	check_path("$ENV{ProgramFiles} (x86)/OpenAL 1.1 SDK/include" p)
	set(OPENAL_INCLUDE_DIR "${p}" CACHE PATH "OpenAL include directory")
	check_path("$ENV{ProgramFiles} (x86)/OpenAL 1.1 SDK/libs/Win64/OpenAL32.lib" p)
	set(OPENAL_LIB_PATH "${p}" CACHE FILEPATH "OpenAL LIB path")
	if(NOT EXISTS "${OPENAL_INCLUDE_DIR}" OR NOT EXISTS "${OPENAL_LIB_PATH}")
		message(STATUS "optional OpenAL is invalid, turn off USE_OPENAL")
		set(USE_OPENAL off)
	endif()
endif()

set(USE_MYSQL on CACHE BOOL "Use MySQL")
if(USE_MYSQL)
	check_path("$ENV{ProgramFiles}/MySQL/MySQL Server 8.0/include" p)
	set(MYSQL_INCLUDE_DIR "${p}" CACHE PATH "MySQL include directory")
	check_path("$ENV{ProgramFiles}/MySQL/MySQL Server 8.0/lib/libmysql.lib" p)
	set(MYSQL_LIB_PATH "${p}" CACHE FILEPATH "MySQL LIB path")
	check_path("$ENV{ProgramFiles}/MySQL/MySQL Server 8.0/lib/libmysql.dll" p)
	set(MYSQL_DLL_PATH "${p}" CACHE FILEPATH "MySQL DLL Path")
	if(NOT EXISTS "${MYSQL_INCLUDE_DIR}" OR NOT EXISTS "${MYSQL_LIB_PATH}" OR NOT EXISTS "${MYSQL_DLL_PATH}")
		message(STATUS "optional MySQL is invalid, turn off USE_MYSQL")
		set(USE_MYSQL off)
	endif()
endif()

check_path("C:\\Program Files\\RenderDoc" p)
set(RENDERDOC_DIR "${p}" CACHE PATH "RenderDoc directory")
if(NOT EXISTS "${RENDERDOC_DIR}")
	set(HAS_RENDERDOC off)
else()
	set(HAS_RENDERDOC on)
endif()

set(USE_UNIVERSE_MODULE on CACHE BOOL "Use Flame Universe Module")

add_subdirectory(source)
add_subdirectory(tests)
add_subdirectory(tools)
