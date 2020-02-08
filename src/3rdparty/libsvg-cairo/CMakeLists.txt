project(C CXX)

aux_source_directory(src SRCS_CAIRO_SVG)
include_directories(include src
	${CMAKE_SOURCE_DIR}/include/cairo
	)
#configure_file(src/svg-cairo-version.h.in ${PROJECT_SOURCE_DIR}/svg-cairo.version.h) 
add_library(cairosvg STATIC ${SRCS_CAIRO_SVG})
install(TARGETS cairosvg DESTINATION lib)

