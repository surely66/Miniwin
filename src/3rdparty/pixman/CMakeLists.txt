project(pixman)
set(PACKAGE pixman)
cmake_minimum_required(VERSION 2.8.8)

set(CMAKE_BUILD_TYPE Release)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3")

# detect version
file (READ configure.ac configure_ac)
string (REGEX REPLACE ".*pixman_major], ([0-9]+).*" "\\1" PIXMAN_VERSION_MAJOR ${configure_ac})
string (REGEX REPLACE ".*pixman_minor], ([0-9]+).*" "\\1" PIXMAN_VERSION_MINOR ${configure_ac})
string (REGEX REPLACE ".*pixman_micro], ([0-9]+).*" "\\1" PIXMAN_VERSION_MICRO ${configure_ac})
set(PIXMAN_VERSION "${PIXMAN_VERSION_MAJOR}.${PIXMAN_VERSION_MINOR}.${PIXMAN_VERSION_MICRO}")

if(NOT MSVC)
    option(BUILD_SHARED "build shared library" ON)
endif()
option(BUILD_STATIC "build static library" OFF)
if(BUILD_STATIC)
    option(BUILD_TESTS "build tests" OFF)
endif()

set(CMAKE_DEBUG_POSTFIX "d")

#dependencies
find_package(Threads)
if(BUILD_TESTS)
    find_package(PNG)
    find_package(OpenMP)
endif()

#arch optimizations
include(${PROJECT_SOURCE_DIR}/cmake/arch_detect.cmake)
include(${PROJECT_SOURCE_DIR}/cmake/arch_configure.cmake)

#config
include(${PROJECT_SOURCE_DIR}/cmake/config_configure.cmake)
message("PROJECT_SOURCE_DIR=${PROJECT_SOURCE_DIR}  PROJECT_BINARY_DIR=${PROJECT_BINARY_DIR}" )
include_directories(
    pixman
    ${PROJECT_BINARY_DIR}/pixman
)

if(MSVC)
	set(CMAKE_C_FLAGS "/wd4244 /wd4146 ${CMAKE_C_FLAGS}")
endif()

if(BUILD_SHARED OR BUILD_STATIC)
    add_subdirectory(pixman)
endif()

if(BUILD_TESTS)
    add_subdirectory(test)
endif(BUILD_TESTS)

include(${PROJECT_SOURCE_DIR}/cmake/cmake_package.cmake)

