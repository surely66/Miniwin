project(gui C CXX)

add_subdirectory(tests)

option( LUA_BINDINGS "UI Lua script bindings" OFF)
option( GUI_STATIC "UI is static link" ON)

aux_source_directory(core SRCS_GUICORE)
#aux_source_directory(core/svg SRCS_SVG)
aux_source_directory(core/cairomm   SRCS_CAIROMM)
aux_source_directory(looper SRCS_LOOP)
message("UI Lua script bindings is ${LUA_BINDINGS} guistatic=${GUI_STATIC}")

if ( LUA_BINDINGS )
    aux_source_directory(luabinding SRCS_LUABIND)
    list(APPEND EXT_LIBS lua)
endif ()

if ( GUI_STATIC )
   list(APPEND EXT_LIBS tvhal_static)
else()
   list(APPEND EXT_LIBS tvhal)
endif()

include_directories(
    ./ ./core ./core/svg ./luabinding 
    ${CMAKE_BINARY_DIR}/include
    ${CMAKE_BINARY_DIR}/include/pixman
    ${CMAKE_BINARY_DIR}/include/cairo
    ${CMAKE_BINARY_DIR}/include/porting
    ${CMAKE_SOURCE_DIR}/src/3rdparty/libunibreak/src
    ${CMAKE_SOURCE_DIR}/src/modules
)

add_library(gui_objs OBJECT ${SRCS_GUICORE} ${SRCS_CAIROMM} ${SRCS_LUABIND} ${SRCS_LOOP} ${SRCS_SVG})
add_library(gui_static STATIC $<TARGET_OBJECTS:gui_objs>)
add_library(gui SHARED $<TARGET_OBJECTS:gui_objs>)
set_target_properties(gui_static PROPERTIES OUTPUT_NAME "gui")

file(GLOB MMHEADERS   ${PROJECT_SOURCE_DIR}/core/cairomm/*.h)
file(GLOB GUIHEADERS  ${PROJECT_SOURCE_DIR}/core/*.h)
file(GLOB LOOPERHEADERS  ${PROJECT_SOURCE_DIR}/looper/*.h)

target_link_libraries(gui  
      pthread pixman-1 png jpeg turbojpeg cairo freetype unibreak usbmanager
      ${EXT_LIBS})
target_link_libraries(gui_static  
      pthread pixman-1 png jpeg turbojpeg cairo_static freetype unibreak usbmanager
      ${EXT_LIBS})

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

