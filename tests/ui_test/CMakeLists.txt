generate_config()
file(GLOB_RECURSE source_files "*.c*")
add_executable(ui_test ${source_files})
set_target_properties(ui_test PROPERTIES FOLDER "tests")
target_link_libraries(ui_test flame_universe)