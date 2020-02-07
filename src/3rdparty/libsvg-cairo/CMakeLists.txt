project(C CXX)

aux_source_directory(src SRCS_CAIRO_SVG)
include_directories(include src
	${CMAKE_BINARY_DIR}/src/3rdparty/cairo-1.16.0
	${CMAKE_SOURCE_DIR}/src/3rdparty/cairo-1.16.0
	)
#configure_file(src/svg-cairo-version.h.in ${PROJECT_SOURCE_DIR}/svg-cairo.version.h) 
add_library(cairosvg STATIC ${SRCS_CAIRO_SVG})
install(TARGETS cairosvg DESTINATION lib)

