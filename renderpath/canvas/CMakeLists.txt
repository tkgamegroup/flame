cmake_minimum_required(VERSION 3.4)

foreach(c ${CMAKE_CONFIGURATION_TYPES})
	string(TOUPPER ${c} c )
	set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${c} ${CMAKE_SOURCE_DIR})
	set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${c} ${CMAKE_SOURCE_DIR})
	set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${c} ${CMAKE_SOURCE_DIR})
endforeach()

project(bp)

add_definitions(-W0 -std:c++latest)

add_library(bp SHARED template.cpp make_cmd.cpp)
target_include_directories(bp PRIVATE ${CMAKE_SOURCE_DIR}/../../include)
target_link_libraries(bp ${CMAKE_SOURCE_DIR}/../../bin/flame_foundation.lib)
target_link_libraries(bp ${CMAKE_SOURCE_DIR}/../../bin/flame_graphics.lib)
