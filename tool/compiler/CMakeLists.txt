file(GLOB_RECURSE SOURCE_LIST "*.c*")
add_executable(compiler ${SOURCE_LIST})
set_target_properties(compiler PROPERTIES FOLDER "tool")
set_target_properties(compiler PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")
target_link_libraries(compiler flame_foundation)

