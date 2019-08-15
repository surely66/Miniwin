project(svg C CXX)

aux_source_directory(./src SRCS_SVG)
set(CMAKE_BUILD_TYPE Debug)
include_directories(./ ./src
  ${CMAKE_SOURCE_DIR}/src/expat/lib
  ${CMAKE_SOURCE_DIR}/src/3rdparty/uriparser/include
  ${CMAKE_SOURCE_DIR}/src/3rdparty/curl/include
)

set(CMAKE_C_FLAGS "-D__DO_SVG_DEBUG=1 -DDEBUG -O0 -g2 -fPIC")

add_library(svg STATIC ${SRCS_SVG})
target_link_libraries(svg expat curl uriparser png)

install(TARGETS svg DESTINATION lib)

