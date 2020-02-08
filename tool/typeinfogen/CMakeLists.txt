file(GLOB_RECURSE SOURCE_LIST "*.c*")
add_executable(typeinfogen ${SOURCE_LIST})
set_target_properties(typeinfogen PROPERTIES FOLDER "tool")
set_target_properties(typeinfogen PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")
target_link_libraries(typeinfogen flame_type)
target_link_libraries(typeinfogen imagehlp.lib)

