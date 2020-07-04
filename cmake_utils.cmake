function(add_typeinfo_support t inc_d src_d)
	add_dependencies(${t} typeinfogen)
	add_custom_command(TARGET ${t} PRE_BUILD COMMAND $<TARGET_FILE:typeinfogen> $<TARGET_FILE:${t}> -c)
	add_custom_command(TARGET ${t} POST_BUILD COMMAND $<TARGET_FILE:typeinfogen> $<TARGET_FILE:${t}> -d${inc_d} -d${src_d})
endfunction()

function(generate_config)
	file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/config.ini 
		"developing = 1\n"
		"resource_path = \"${CMAKE_CURRENT_SOURCE_DIR}\"\n"
		"engine_path = {e}\n"
	)
endfunction()
