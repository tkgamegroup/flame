function(check_path p out)
    if(EXISTS "${p}")
        set(${out} "${p}" PARENT_SCOPE)
    else()
        set(${out} "" PARENT_SCOPE)
    endif()
endfunction()

function(assign_source_group parent_path folder_prefix)
    foreach(_source IN ITEMS ${ARGN})
        if (IS_ABSOLUTE "${_source}")
            file(RELATIVE_PATH _source_rel "${parent_path}" "${_source}")
        else()
            set(_source_rel "${_source}")
        endif()
        get_filename_component(_source_path "${_source_rel}" PATH)
        string(REPLACE "/" "\\" _source_path_msvc "${_source_path}")
        source_group("${folder_prefix}${_source_path_msvc}" FILES "${_source}")
    endforeach()
endfunction()

function(generate_typedesc tar desc)
    file(GENERATE OUTPUT "$<TARGET_FILE_DIR:${tar}>/${tar}.typedesc" CONTENT "${CMAKE_CURRENT_SOURCE_DIR}\n${desc}" TARGET ${tar} )
endfunction()

function(gen_typeinfo tar1 tar2)
    add_custom_command(TARGET ${tar1} POST_BUILD COMMAND $<TARGET_FILE:typeinfogen> $<TARGET_FILE:${tar2}>)
endfunction()

function(add_typeinfo_support tar desc)
    add_dependencies(${tar} typeinfogen_dep)
    generate_typedesc(${tar} ${desc})
    gen_typeinfo(${tar} ${tar})
endfunction()

function(generate_config)
	file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/config.ini 
		"developing = 1\n"
		"resource_path = \"${CMAKE_CURRENT_SOURCE_DIR}\"\n"
		"engine_path = {e}\n"
	)
endfunction()
