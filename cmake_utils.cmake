function(add_typeinfo_support t d)
	add_dependencies(${t} typeinfogen)
	add_custom_command(TARGET ${t} PRE_BUILD COMMAND powershell -command Remove-Item $<TARGET_PDB_FILE:${t}> -ErrorAction Ignore & set errorlevel=0)
	add_custom_command(TARGET ${t} POST_BUILD COMMAND $<TARGET_FILE:typeinfogen> -i $<TARGET_FILE:${t}> -d ${d})
endfunction()

function(generate_config)
	file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/config.ini 
		"developing = 1\n"
		"resource_path = \"${CMAKE_CURRENT_SOURCE_DIR}\"\n"
		"engine_path = {e}\n"
	)
endfunction()
