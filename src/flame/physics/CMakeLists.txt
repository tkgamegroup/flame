file(GLOB HEADER_LIST "*.h*")
file(GLOB SOURCE_LIST "*.c*")
add_lib(flame_physics "${HEADER_LIST}" "${SOURCE_LIST}" "" "flame")
target_compile_definitions(flame_physics PRIVATE FLAME_PHYSICS_MODULE)
target_include_directories(flame_physics PUBLIC "${CMAKE_SOURCE_DIR}/src")
target_include_directories(flame_physics PUBLIC "${CMAKE_SOURCE_DIR}/ext/Physx-3.4/PhysX_3.4/Include")
target_include_directories(flame_physics PUBLIC "${CMAKE_SOURCE_DIR}/ext/Physx-3.4/PxShared/include")
target_link_libraries(flame_physics ${CMAKE_SOURCE_DIR}/ext/Physx-3.4/PhysX_3.4/Lib/vc15win${FLAME_SYS_BIT}/PhysX3DEBUG_${FLAME_SYS_NAME}.lib)
target_link_libraries(flame_physics ${CMAKE_SOURCE_DIR}/ext/Physx-3.4/PhysX_3.4/Lib/vc15win${FLAME_SYS_BIT}/PhysX3CommonDEBUG_${FLAME_SYS_NAME}.lib)
target_link_libraries(flame_physics ${CMAKE_SOURCE_DIR}/ext/Physx-3.4/PhysX_3.4/Lib/vc15win${FLAME_SYS_BIT}/PhysX3ExtensionsDEBUG.lib)
target_link_libraries(flame_physics ${CMAKE_SOURCE_DIR}/ext/Physx-3.4/PxShared/lib/vc15win${FLAME_SYS_BIT}/PxFoundationDEBUG_${FLAME_SYS_NAME}.lib)
add_custom_target(copy_physx_dlls 
	COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_SOURCE_DIR}/ext/Physx-3.4/PhysX_3.4/Bin/vc15win${FLAME_SYS_BIT}/PhysX3CommonDEBUG_${FLAME_SYS_NAME}.dll ${CMAKE_SOURCE_DIR}/bin
	COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_SOURCE_DIR}/ext/Physx-3.4/PhysX_3.4/Bin/vc15win${FLAME_SYS_BIT}/PhysX3DEBUG_${FLAME_SYS_NAME}.dll ${CMAKE_SOURCE_DIR}/bin
	COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_SOURCE_DIR}/ext/Physx-3.4/PxShared/bin/vc15win${FLAME_SYS_BIT}/PxFoundationDEBUG_${FLAME_SYS_NAME}.dll ${CMAKE_SOURCE_DIR}/bin
)
add_dependencies(flame_physics copy_physx_dlls)
