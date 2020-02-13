project(gui C CXX)

add_subdirectory(tests)

option( LUA_BINDINGS "UI Lua script bindings" OFF)

#set(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS}  -fPIC")
#set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -fPIC")

aux_source_directory(core SRCS_GUICORE)
#aux_source_directory(core/svg SRCS_SVG)
aux_source_directory(core/cairomm   SRCS_CAIROMM)
aux_source_directory(looper SRCS_LOOP)
message("UI Lua script bindings is ${LUA_BINDINGS}")
if ( LUA_BINDINGS )
aux_source_directory(luabinding SRCS_LUABIND)
list(APPEND EXT_LIBS lua)
endif ()

include_directories(
    ./ ./core ./core/svg ./luabinding 
    ${CMAKE_BINARY_DIR}/include
    ${CMAKE_BINARY_DIR}/include/pixman
    ${CMAKE_BINARY_DIR}/include/cairo
    ${CMAKE_BINARY_DIR}/include/porting
    ${CMAKE_SOURCE_DIR}/src/3rdparty/libsvg/src/
    ${CMAKE_SOURCE_DIR}/src/3rdparty/libsvgtiny/include
    ${CMAKE_SOURCE_DIR}/src/3rdparty/simple-svg/include
    ${CMAKE_SOURCE_DIR}/src/3rdparty/simple-svg/bx/include
    ${CMAKE_SOURCE_DIR}/src/3rdparty/libsvg-cairo/include
    ${CMAKE_SOURCE_DIR}/src/3rdparty/libunibreak/src
    ${CMAKE_SOURCE_DIR}/src/modules
)

add_library(gui SHARED ${SRCS_GUICORE} ${SRCS_CAIROMM} ${SRCS_LUABIND} ${SRCS_LOOP} ${SRCS_SVG})

file(GLOB MMHEADERS   ${PROJECT_SOURCE_DIR}/core/cairomm/*.h)
file(GLOB GUIHEADERS  ${PROJECT_SOURCE_DIR}/core/*.h)
file(GLOB LOOPERHEADERS  ${PROJECT_SOURCE_DIR}/looper/*.h)

link_directories(
    ${CMAKE_BINARY_DIR}/src/3rdparty/libpng/
    ${CMAKE_BINARY_DIR}/src/3rdparty/lua 
    ${CMAKE_BINARY_DIR}/lib)

target_link_libraries(gui  pthread pixman-1 png jpeg cairo freetype
       	tvhal jsoncpp_lib unibreak usbmanager ${EXT_LIBS}
	#svg cairosvg
)

# simplesvg)

add_executable(respackt core/filepak.cc core/gzstream.cc)
target_compile_definitions(respackt PRIVATE -DPACK_TOOL)
target_link_libraries(respackt zlib)
 
file(COPY ${GUIHEADERS}    DESTINATION ${CMAKE_BINARY_DIR}/include/gui PATTERN "*.h")
file(COPY ${MMHEADERS}     DESTINATION ${CMAKE_BINARY_DIR}/include/gui/cairomm PATTERN "*.h")
file(COPY ${LOOPERHEADERS} DESTINATION ${CMAKE_BINARY_DIR}/include/looper PATTERN "*.h")

install (TARGETS gui  DESTINATION lib)
install (TARGETS respackt  DESTINATION bin)
install (FILES ${MMHEADERS} DESTINATION include/gui/cairomm/)
install (FILES ${GUIHEADERS} DESTINATION include/gui/)
install (FILES ${LOOPERHEADERS} DESTINATION include/looper)

