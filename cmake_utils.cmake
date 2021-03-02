function(add_typeinfo_support t d)
	add_dependencies(${t} typeinfogen)
	if(${t} STREQUAL "typeinfogen")
		set(T "flame_foundation")
	else()
		set(T ${t})
		add_custom_command(TARGET ${t} PRE_BUILD COMMAND powershell -command Remove-Item $<TARGET_PDB_FILE:${T}> -ErrorAction Ignore & set errorlevel=0)
	endif()
	add_custom_command(TARGET ${t} POST_BUILD COMMAND $<TARGET_FILE:typeinfogen> -i $<TARGET_FILE:${T}> -d "${d}/typeinfo.desc")
endfunction()

function(generate_config)
	file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/config.ini 
		"developing = 1\n"
		"resource_path = \"${CMAKE_CURRENT_SOURCE_DIR}\"\n"
		"engine_path = {e}\n"
	)
endfunction()
