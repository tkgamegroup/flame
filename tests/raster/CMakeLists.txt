file(GLOB_RECURSE SOURCE_LIST "*.c*")
add_executable(raster ${SOURCE_LIST})
set_target_properties(raster PROPERTIES FOLDER "tests")
target_link_libraries(raster flame_foundation)
