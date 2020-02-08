file(GLOB_RECURSE SOURCE_LIST "*.c*")
add_executable(when_changed ${SOURCE_LIST})
set_target_properties(when_changed PROPERTIES FOLDER "tool")
set_target_properties(when_changed PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")
target_link_libraries(when_changed flame_foundation)

