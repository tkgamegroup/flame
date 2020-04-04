add_executable(empty_test "empty_test.cpp")
set_target_properties(empty_test PROPERTIES FOLDER "test")

add_executable(window_test "window_test.cpp")
set_target_properties(window_test PROPERTIES FOLDER "test")
target_link_libraries(window_test flame_foundation)

add_executable(8beeper "8beeper.cpp")
set_target_properties(8beeper PROPERTIES FOLDER "test")
target_link_libraries(8beeper flame_sound)

add_executable(graphics_test_clear "graphics_test_clear.cpp")
set_target_properties(graphics_test_clear PROPERTIES FOLDER "test")
target_link_libraries(graphics_test_clear flame_graphics)

add_executable(graphics_test_image "graphics_test_image.cpp")
set_target_properties(graphics_test_image PROPERTIES FOLDER "test")
target_link_libraries(graphics_test_image flame_graphics)
add_dependencies(graphics_test_image typeinfogen)

add_executable(graphics_test_bp "graphics_test_bp.cpp")
set_target_properties(graphics_test_bp PROPERTIES FOLDER "test")
target_link_libraries(graphics_test_bp flame_graphics)
add_dependencies(graphics_test_bp typeinfogen)
add_dependencies(graphics_test_bp compiler)

add_executable(graphics_test_canvas "graphics_test_canvas.cpp")
set_target_properties(graphics_test_canvas PROPERTIES FOLDER "test")
target_link_libraries(graphics_test_canvas flame_graphics)
add_dependencies(graphics_test_canvas typeinfogen)
add_dependencies(graphics_test_canvas compiler)

add_executable(sound_test "sound_test.cpp")
set_target_properties(sound_test PROPERTIES FOLDER "test")
target_link_libraries(sound_test flame_sound)

add_executable(ui_test_cpp "ui_test_cpp.cpp")
set_target_properties(ui_test_cpp PROPERTIES FOLDER "test")
target_link_libraries(ui_test_cpp flame_universe)
target_link_libraries(ui_test_cpp flame_sound)
add_dependencies(ui_test_cpp typeinfogen)
add_dependencies(ui_test_cpp compiler)

add_executable(ui_test_file "ui_test_file.cpp")
set_target_properties(ui_test_file PROPERTIES FOLDER "test")
target_link_libraries(ui_test_file flame_universe)
target_link_libraries(ui_test_file flame_sound)
add_dependencies(ui_test_file typeinfogen)
add_dependencies(ui_test_file compiler)

