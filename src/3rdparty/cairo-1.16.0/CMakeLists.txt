project(cairo)
cmake_minimum_required(VERSION 2.8)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

#find_package(pixman REQUIRED)
find_package(PNG )
find_package(ZLIB)
find_package(freetype)
find_package(Threads)
#find_package(Fontconfig)
#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3 -Wno-attributes -fPIC")

if ( NOT ZLIB_FOUND )
set(ZLIB_LIBRARY zlib)
set(ZLIB_INCLUDE_DIR ${CMAKE_BINARY_DIR}/include)
set(ZLIB_FOUND TRUE)
endif()

if (NOT PNG_FOUND )
set(PNG_LIBRARY png)
set(PNG_INCLUDE_DIR ${CMAKE_BINARY_DIR}/include)
set(CAIRO_HAS_PNG_FUNCTIONS TRUE)
set(PNG_FOUND TRUE)
endif()

if(NOT PIXMAN_FOUND)
set(PIXMAN_LIBRARY pixman-1)
set(PIXMAN_INCLUDE_DIR ${CMAKE_BINARY_DIR}/include)
set(PIXMAN_FOUND TRUE)
endif()

include(Configure_config.cmake)
include(Configure_features.cmake)
include_directories(
   ${cairo_BINARY_DIR}
   ${CMAKE_BINARY_DIR}/include/
   ${CMAKE_BINARY_DIR}/include/porting
   ${CMAKE_BINARY_DIR}/include/pixman
)

if(CAIRO_HAS_PTHREAD)
    list(APPEND CAIRO_LIBS -lpthread)
endif()

if (WIN32)
    set(CAIRO_LIBS gdi32 msimg32 user32 winmm)
endif()

include_directories(${PIXMAN_INCLUDE_DIR} ${PNG_INCLUDE_DIR} ${ZLIB_INCLUDE_DIR})
list(APPEND CAIRO_LIBS  ${PIXMAN_LIBRARY} ${PNG_LIBRARY} ${ZLIB_LIBRARY})

if(FONTCONFIG_FOUND)
    list(APPEND CAIRO_LIBS ${FONTCONFIG_LIBRARY})
endif()

if(FREETYPE_FOUND)
#    find_package(BZip2)
    include_directories(${FREETYPE_INCLUDE_DIRS})
#    include_directories(${BZIP2_INCLUDE_DIR})
    list(APPEND CAIRO_LIBS ${FREETYPE_LIBRARIES} ${BZIP2_LIBRARIES})
else()
    list(APPEND CAIRO_LIBS freetype zlib)
endif()

include_directories(src)
add_subdirectory(src)
