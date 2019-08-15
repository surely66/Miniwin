cmake_minimum_required (VERSION 3.2)

project(alitest C CXX)

set(CMAKE_BUILD_TYPE "Debug")

set(CMAKE_CXX_STANDARD 11)

#message("MEMWATCH used")
#add_definitions(-DMEMWATCH=1)

set(ENV{NGL_CHIP_PLATFORM} ali3528)
set(ENV{CAS_SYSTEM_NAME} ideto)

add_subdirectory(src/gui)
add_subdirectory(src/modules)
add_subdirectory(src/acs)
add_subdirectory(src/epg)
add_subdirectory(src/porting)
add_subdirectory(src/apps)
add_subdirectory(src/3rdparty)

