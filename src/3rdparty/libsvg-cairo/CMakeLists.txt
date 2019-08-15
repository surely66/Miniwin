project(C CXX)

aux_source_directory(src SRCS_CAIRO_SVG)
include_directories(include src)

add_library(cairosvg STATIC ${SRCS_CAIRO_SVG})
install(TARGETS cairosvg DESTINATION lib)

