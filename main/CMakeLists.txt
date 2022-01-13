project( main LANGUAGES C )

add_executable( main
	main.c )

set_target_properties( main PROPERTIES
	OUTPUT_NAME ${PROGRAM_NAME}
	C_STANDARD 11
	C_STANDARD_REQUIRED TRUE )

target_link_libraries( main
	PRIVATE
	gui )

install( TARGETS main
	RUNTIME )
