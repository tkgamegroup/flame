generate_config()
file(GLOB_RECURSE source_files "*.h" "*.c*")
list(REMOVE_ITEM source_files "${CMAKE_CURRENT_SOURCE_DIR}/vs_automation.cpp")

set(CUSTOM_COMPILE_FLAGS "${CMAKE_CXX_FLAGS}")
string(REPLACE "/RTC1" "" CUSTOM_COMPILE_FLAGS ${CUSTOM_COMPILE_FLAGS})
string(REPLACE "/EHsc" "" CUSTOM_COMPILE_FLAGS ${CUSTOM_COMPILE_FLAGS})
separate_arguments(CUSTOM_COMPILE_FLAGS NATIVE_COMMAND ${CUSTOM_COMPILE_FLAGS})
add_custom_command(OUTPUT vs_automation.o COMMAND ${CMAKE_CXX_COMPILER} ARGS ${CUSTOM_COMPILE_FLAGS} /MDd /clr /AI$(MSBuildFrameworkToolsPath64) /AI${CMAKE_SOURCE_DIR}/bin 
 -c "${CMAKE_CURRENT_SOURCE_DIR}/vs_automation.cpp" -Fo"${CMAKE_CURRENT_BINARY_DIR}/vs_automation.o" MAIN_DEPENDENCY vs_automation.cpp)

add_executable(scene_editor "${source_files};vs_automation.o")
set_target_properties(scene_editor PROPERTIES FOLDER "tools")
target_link_options(scene_editor PRIVATE "/FIXED:NO")
target_link_libraries(scene_editor flame_universe)
target_link_libraries(scene_editor vs_automation)
set_target_properties(scene_editor PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")
