
project(dvbepg C CXX)

set(CMAKE_CXX_STANDARD 11)

aux_source_directory(./ SRCS_EPG)
add_definitions(-DLIBICONV_PLUG)
add_library(dvbepg SHARED ${SRCS_EPG})
include_directories(
   ./
   ${CMAKE_BINARY_DIR}/include
   ${CMAKE_BINARY_DIR}/include/porting
   ${CMAKE_BINARY_DIR}/include/jsoncpp
   ../
)

target_link_libraries(dvbepg tvhal iconv jsoncpp_lib dl)

#link_directories(${CMAKE_BINARY_DIR}/src/3rdparty/libiconv)

add_custom_command(TARGET dvbepg POST_BUILD
    COMMAND mkdir -p ${CMAKE_SOURCE_DIR}/include/epg
    COMMAND cp -u *.h  ${CMAKE_SOURCE_DIR}/include/epg/
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
    COMMENT "Copy EPG Header Files")

add_subdirectory(tests)

install (TARGETS dvbepg  DESTINATION lib)
