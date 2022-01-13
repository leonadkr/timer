project( gui LANGUAGES C )

find_package( PkgConfig REQUIRED )
pkg_check_modules( GTK3 REQUIRED gtk+-3.0 )

add_library( gui
	STATIC
	window.c
	gui.c )

set_target_properties( gui PROPERTIES
	C_STANDARD 11
	C_STANDARD_REQUIRED TRUE )

target_include_directories( gui
	PRIVATE
	${GTK3_INCLUDE_DIRS}
	PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR} )

target_link_directories( gui
	PRIVATE
	${GTK3_LIBRARY_DIRS} )

target_link_libraries( gui
	PRIVATE
	${GTK3_LIBRARIES} )
