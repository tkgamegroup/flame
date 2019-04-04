add_exe(window_test "test_window.cpp" "test")
target_link_libraries(window_test flame_foundation)

add_exe(graphics_test "test_graphics.cpp" "test")
target_link_libraries(graphics_test flame_graphics)

add_exe(ui_test "test_ui.cpp" "test")
target_link_libraries(ui_test flame_universe)

add_exe(dx_test "test_dx.cpp" "test")
target_link_libraries(dx_test flame_foundation)
target_link_libraries(dx_test "d3d12.lib" "dxgi.lib")
