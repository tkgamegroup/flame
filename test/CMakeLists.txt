add_executable(window_test "window_test.cpp")
set_target_properties(window_test PROPERTIES FOLDER "test")
set_target_properties(window_test PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")
target_link_libraries(window_test flame_foundation)

add_executable(graphics_test_bp "graphics_test_bp.cpp")
set_target_properties(graphics_test_bp PROPERTIES FOLDER "test")
set_target_properties(graphics_test_bp PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")
target_link_libraries(graphics_test_bp flame_graphics)

add_executable(graphics_test_canvas "graphics_test_canvas.cpp")
set_target_properties(graphics_test_canvas PROPERTIES FOLDER "test")
set_target_properties(graphics_test_canvas PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")
target_link_libraries(graphics_test_canvas flame_graphics)

add_executable(sound_test "sound_test.cpp")
set_target_properties(sound_test PROPERTIES FOLDER "test")
set_target_properties(sound_test PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")
target_link_libraries(sound_test flame_sound)

add_executable(ui_test_cpp "ui_test_cpp.cpp")
set_target_properties(ui_test_cpp PROPERTIES FOLDER "test")
set_target_properties(ui_test_cpp PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")
target_link_libraries(ui_test_cpp flame_universe)

add_executable(ui_test_file "ui_test_file.cpp")
set_target_properties(ui_test_file PROPERTIES FOLDER "test")
set_target_properties(ui_test_file PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")
target_link_libraries(ui_test_file flame_universe)

