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

include(do.cmake)

