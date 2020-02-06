
project(dvbepg C CXX)

set(CMAKE_CXX_STANDARD 11)

#set(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -fPIC")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")

aux_source_directory(./ SRCS_EPG)
add_definitions(-DLIBICONV_PLUG)
add_library(dvbepg SHARED ${SRCS_EPG})
include_directories(
   ./
   ${CMAKE_SOURCE_DIR}/src/3rdparty/libiconv/include
   ${CMAKE_BINARY_DIR}/src/3rdparty/libiconv/include
   ${CMAKE_SOURCE_DIR}/include
   ${CMAKE_SOURCE_DIR}/include/porting
   ${CMAKE_SOURCE_DIR}/src/3rdparty/jsoncpp/include
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
