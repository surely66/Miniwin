project(gui C CXX)

add_subdirectory(tests)
add_subdirectory(tools)

aux_source_directory(core SRCS_GUICORE)
aux_source_directory(core/svg SRCS_SVG)
aux_source_directory(core/cairomm   SRCS_CAIROMM)

add_library(gui SHARED ${SRCS_GUICORE} ${SRCS_CAIROMM} ${SRCS_SVG})

include_directories(
    ./core ./core/svg 
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/include/porting
    ${CMAKE_SOURCE_DIR}/include/cairo
    ${CMAKE_SOURCE_DIR}/src/3rdparty/libjpeg/
    ${CMAKE_SOURCE_DIR}/src/3rdparty/libpng/
    ${CMAKE_SOURCE_DIR}/src/3rdparty/libsvg/src/
    ${CMAKE_SOURCE_DIR}/src/3rdparty/libsvgtiny/include
    ${CMAKE_SOURCE_DIR}/src/3rdparty/simple-svg/include
    ${CMAKE_SOURCE_DIR}/src/3rdparty/simple-svg/bx/include
    ${CMAKE_SOURCE_DIR}/src/3rdparty/libsvg-cairo/include
    ${CMAKE_SOURCE_DIR}/src/3rdparty/jsoncpp/include
    ${CMAKE_SOURCE_DIR}/src/3rdparty/libunibreak/src
    ${CMAKE_SOURCE_DIR}/src/modules
)


file(GLOB MMHEADERS   ${PROJECT_SOURCE_DIR}/core/cairomm/*.h)
file(GLOB GUIHEADERS  ${PROJECT_SOURCE_DIR}/core/*.h)

target_link_libraries(gui  pthread pixman-1 png jpeg cairo-static freetype tvhal jsoncpp_lib unibreak svg usbmanager cairosvg)
# simplesvg)

#add_custom_command(TARGET gui PRE_BUILD 
#           COMMAND g++ filepak.cc gzstream.cc -std=c++11 -I./ -lz -DPACK_TOOL -o ${CMAKE_SOURCE_DIR}/respack 
#           WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/core
#           COMMENT "compile resource package tools")

add_executable(respackt core/filepak.cc core/gzstream.cc)
target_compile_definitions(respackt PRIVATE -DPACK_TOOL)
target_link_libraries(respackt z)
 
add_custom_command(TARGET gui POST_BUILD 
    COMMAND mkdir -p ${CMAKE_SOURCE_DIR}/include/gui/cairomm
    COMMAND cp -u *.h  ${CMAKE_SOURCE_DIR}/include/gui/   
    COMMAND cp -u cairomm/*.h  ${CMAKE_SOURCE_DIR}/include/gui/cairomm   
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/core
    COMMENT "Copy GUI Header Files")
add_executable(svgtest core/svgtest.c)
target_link_libraries(svgtest jpeg png svg)

install (TARGETS gui  DESTINATION lib)
install (TARGETS svgtest  DESTINATION bin)
install (TARGETS respackt  DESTINATION bin)
install (FILES ${MMHEADERS} DESTINATION include/gui/cairomm/)
install (FILES ${GUIHEADERS} DESTINATION include/gui/)

