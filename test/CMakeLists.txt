add_exe(window_test "window_test.cpp" "test")
target_link_libraries(window_test flame_foundation)

add_exe(graphics_test "graphics_test.cpp" "test")
target_link_libraries(graphics_test flame_graphics)

add_exe(sound_test "sound_test.cpp" "test")
target_link_libraries(sound_test flame_sound)

add_exe(ui_test "ui_test.cpp" "test")
target_link_libraries(ui_test flame_universe)

