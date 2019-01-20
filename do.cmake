project(flame)

include(utils.cmake)

option(FLAME_ENABLE_PHYSICS "FLAME_ENABLE_PHYSICS" OFF)

set_output_dir("${CMAKE_SOURCE_DIR}/bin")

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
add_subdirectory(tests)
add_subdirectory(games)
add_subdirectory(tools)
