
project(dvbepg C CXX)

set(CMAKE_CXX_STANDARD 11)

aux_source_directory(./ SRCS_EPG)
add_definitions(-DLIBICONV_PLUG)
add_library(dvbepg SHARED ${SRCS_EPG})
include_directories(
   ./
   ${CMAKE_BINARY_DIR}/include
   ${CMAKE_BINARY_DIR}/include/porting
   ${CMAKE_BINARY_DIR}/include/epg
   ../
)
file(GLOB TVHEADERS   ${PROJECT_SOURCE_DIR}/*.h)
target_link_libraries(dvbepg tvhal iconv dl)
file(COPY ${TVHEADERS} DESTINATION ${CMAKE_BINARY_DIR}/include/epg)
add_subdirectory(tests)

install (TARGETS dvbepg  DESTINATION lib)
