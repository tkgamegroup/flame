add_exe(window_test "window_test.cpp" "test")
target_link_libraries(window_test flame_foundation)

add_exe(graphics_test_cpp "graphics_test_cpp.cpp" "test")
target_link_libraries(graphics_test_cpp flame_graphics)
#add_custom_command(TARGET graphics_test_cpp POST_BUILD COMMAND ${CMAKE_SOURCE_DIR}/bin/typeinfogen ${CMAKE_SOURCE_DIR}/bin/graphics_test_cpp.exe ${CMAKE_SOURCE_DIR}/bin/flame_foundation.dll ${CMAKE_SOURCE_DIR}/bin/flame_graphics.dll)

add_exe(graphics_test_bp "graphics_test_bp.cpp" "test")
target_link_libraries(graphics_test_bp flame_graphics)

add_exe(sound_test "sound_test.cpp" "test")
target_link_libraries(sound_test flame_sound)

add_exe(ui_test "ui_test.cpp" "test")
target_link_libraries(ui_test flame_universe)

