project(gui C CXX)

add_subdirectory(tests)
add_subdirectory(tools)

option( LUA_BINDINGS "UI Lua script bindings" OFF)

aux_source_directory(core SRCS_GUICORE)
#aux_source_directory(core/svg SRCS_SVG)
aux_source_directory(core/cairomm   SRCS_CAIROMM)
aux_source_directory(looper SRCS_LOOP)
message("UI Lua script bindings is ${LUA_BINDINGS}")
message("=======CMAKE_BINARY_DIR=${CMAKE_BINARY_DIR}")
if ( LUA_BINDINGS )
aux_source_directory(luabinding SRCS_LUABIND)
list(APPEND EXT_LIBS lua)
endif ()

include_directories(
    ./ ./core ./core/svg ./luabinding 
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/include/pixman
    ${CMAKE_SOURCE_DIR}/include/cairo
    ${CMAKE_SOURCE_DIR}/include/porting
    ${CMAKE_SOURCE_DIR}/src/3rdparty/libsvg/src/
    ${CMAKE_SOURCE_DIR}/src/3rdparty/libsvgtiny/include
    ${CMAKE_SOURCE_DIR}/src/3rdparty/simple-svg/include
    ${CMAKE_SOURCE_DIR}/src/3rdparty/simple-svg/bx/include
    ${CMAKE_SOURCE_DIR}/src/3rdparty/libsvg-cairo/include
    ${CMAKE_SOURCE_DIR}/src/3rdparty/libunibreak/src
    ${CMAKE_SOURCE_DIR}/src/modules
    ${CMAKE_BINARY_DIR}/include
)

add_library(gui SHARED ${SRCS_GUICORE} ${SRCS_CAIROMM} ${SRCS_LUABIND} ${SRCS_LOOP} ${SRCS_SVG})

file(GLOB MMHEADERS   ${PROJECT_SOURCE_DIR}/core/cairomm/*.h)
file(GLOB GUIHEADERS  ${PROJECT_SOURCE_DIR}/core/*.h)
file(GLOB LOOPERHEADERS  ${PROJECT_SOURCE_DIR}/looper/*.h)
message("LOOPERHEADERS=${LOOPERHEADERS}")
link_directories(
    ${CMAKE_BINARY_DIR}/src/3rdparty/libpng/
    ${CMAKE_BINARY_DIR}/src/3rdparty/lua 
    ${CMAKE_BINARY_DIR}/lib)

target_link_libraries(gui  pthread pixman-1 png jpeg cairo-static freetype
       	tvhal jsoncpp_lib unibreak usbmanager ${EXT_LIBS}
	#svg cairosvg
)

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
    COMMAND mkdir -p ${CMAKE_SOURCE_DIR}/include/looper
    COMMAND cp -u *.h  ${CMAKE_SOURCE_DIR}/include/gui/   
    COMMAND cp -u cairomm/*.h  ${CMAKE_SOURCE_DIR}/include/gui/cairomm   
    COMMAND cp -u ../looper/*.h  ${CMAKE_SOURCE_DIR}/include/looper   
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/core
    COMMENT "Copy GUI Header Files")

install (TARGETS gui  DESTINATION lib)
install (TARGETS respackt  DESTINATION bin)
install (FILES ${MMHEADERS} DESTINATION include/gui/cairomm/)
install (FILES ${GUIHEADERS} DESTINATION include/gui/)
install (FILES ${LOOPERHEADERS} DESTINATION include/looper)

