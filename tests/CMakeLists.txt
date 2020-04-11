add_executable(empty_test "empty_test.cpp")
set_target_properties(empty_test PROPERTIES FOLDER "tests")

add_executable(window_test "window_test.cpp")
set_target_properties(window_test PROPERTIES FOLDER "tests")
target_link_libraries(window_test flame_foundation)

add_executable(8beeper "8beeper.cpp")
set_target_properties(8beeper PROPERTIES FOLDER "tests")
target_link_libraries(8beeper flame_sound)

add_executable(graphics_test_clear "graphics_test_clear.cpp")
set_target_properties(graphics_test_clear PROPERTIES FOLDER "tests")
target_link_libraries(graphics_test_clear flame_graphics)

add_executable(graphics_test_copy_image "graphics_test_copy_image.cpp")
set_target_properties(graphics_test_copy_image PROPERTIES FOLDER "tests")
target_link_libraries(graphics_test_copy_image flame_graphics)

add_executable(graphics_test_draw_image "graphics_test_draw_image.cpp")
set_target_properties(graphics_test_draw_image PROPERTIES FOLDER "tests")
target_link_libraries(graphics_test_draw_image flame_graphics)

add_executable(graphics_test_canvas "graphics_test_canvas.cpp")
set_target_properties(graphics_test_canvas PROPERTIES FOLDER "tests")
target_link_libraries(graphics_test_canvas flame_graphics)

add_executable(sound_test "sound_test.cpp")
set_target_properties(sound_test PROPERTIES FOLDER "tests")
target_link_libraries(sound_test flame_sound)

add_executable(ui_test_cpp "ui_test_cpp.cpp")
set_target_properties(ui_test_cpp PROPERTIES FOLDER "tests")
target_link_libraries(ui_test_cpp flame_universe)
target_link_libraries(ui_test_cpp flame_sound)
add_dependencies(ui_test_cpp typeinfogen)

add_executable(ui_test_file "ui_test_file.cpp")
set_target_properties(ui_test_file PROPERTIES FOLDER "tests")
target_link_libraries(ui_test_file flame_universe)
target_link_libraries(ui_test_file flame_sound)
add_dependencies(ui_test_file typeinfogen)

