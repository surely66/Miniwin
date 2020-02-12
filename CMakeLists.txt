cmake_minimum_required (VERSION 3.2)

project(alitest C CXX)

set(CMAKE_BUILD_TYPE "Debug")
set(CMAKE_CXX_STANDARD 11)
set(CAIRO_HAS_PDF_SURFACE TRUE)
set(CAIRO_HAS_SCRIPT_SURFACE TRUE)
set(CAIRO_HAS_PS_SURFACE TRUE)
set(CAIRO_HAS_NGL_SURFACE TRUE)
set(CAIRO_HAS_FT_FONT TRUE)
set(CAIRO_HAS_XML_SURFACE TRUE)
add_subdirectory(src/gui)
add_subdirectory(src/modules)
add_subdirectory(src/porting)
add_subdirectory(src/apps)
add_subdirectory(src/3rdparty)
add_subdirectory(src/tvmodules)

if(NOT CMAKE_CROSSCOMPILING)
  add_subdirectory(src/tools)
endif()

set(CMAKE_EXE_LINKER_FLAGS "-Xlinker --no-allow-shlib-undefined ${CMAKE_EXE_LINKER_FLAGS}")
