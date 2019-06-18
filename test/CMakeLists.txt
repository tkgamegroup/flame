add_exe(window_test "test_window.cpp" "test")
target_link_libraries(window_test flame_foundation)

add_exe(sound_test "test_sound.cpp" "test")
target_link_libraries(sound_test flame_sound)

add_exe(ui_test "test_ui.cpp" "test")
target_link_libraries(ui_test flame_universe)

