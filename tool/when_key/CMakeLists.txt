file(GLOB_RECURSE SOURCE_LIST "*.c*")
add_executable(when_key ${SOURCE_LIST})
set_target_properties(when_key PROPERTIES FOLDER "tool")
set_target_properties(when_key PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")
target_link_libraries(when_key flame_foundation)

