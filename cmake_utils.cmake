function(generate_config)
file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/config.ini 
	"developing = 1\n"
	"resource_path = \"${CMAKE_CURRENT_SOURCE_DIR}\"\n"
	"engine_path = {e}\n"
)
endfunction()
