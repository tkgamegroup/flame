file(GLOB_RECURSE SOURCES "*.c*")
add_executable(raster ${SOURCES})
set_target_properties(raster PROPERTIES FOLDER "tests")
target_link_libraries(raster flame_foundation)
