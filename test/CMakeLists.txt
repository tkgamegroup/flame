add_exe(window_test "window_test.cpp" "test")
target_link_libraries(window_test flame_foundation)

add_exe(graphics_test_cpp "graphics_test_cpp.cpp" "test")
target_link_libraries(graphics_test_cpp flame_graphics)

add_exe(graphics_test_bp "graphics_test_bp.cpp" "test")
target_link_libraries(graphics_test_bp flame_graphics)

add_exe(sound_test "sound_test.cpp" "test")
target_link_libraries(sound_test flame_sound)

add_exe(ui_test_cpp "ui_test_cpp.cpp" "test")
target_link_libraries(ui_test_cpp flame_universe)

add_exe(ui_test_file "ui_test_file.cpp" "test")
target_link_libraries(ui_test_file flame_universe)

