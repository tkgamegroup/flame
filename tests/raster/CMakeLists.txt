file(GLOB_RECURSE source_files "*.c*")
add_executable(raster ${source_files})
set_target_properties(raster PROPERTIES FOLDER "tests")
target_link_libraries(raster flame_foundation)
