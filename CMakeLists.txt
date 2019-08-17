cmake_minimum_required (VERSION 3.2)

project(alitest C CXX)

set(CMAKE_BUILD_TYPE "Debug")

set(CMAKE_CXX_STANDARD 11)

#message("MEMWATCH used")
#add_definitions(-DMEMWATCH=1)

if(CMAKE_SYSTEM_PROCESSOR MATCHES "^x86" OR CMAKE_SYSTEM_PROCESSOR MATCHES "amd" )
    set(NGL_CHIP_PLATFORM "nglx86" CACHE INTERNAL "x86 emulator porting")
else()
    set(CA_PLATFORM "viaccess" CACHE INTERNAL "CA TYPE")
    set(NGL_CHIP_PLATFORM "ali3528" CACHE INTERNAL "defined which porting layer to used")
endif()

add_subdirectory(src/gui)
add_subdirectory(src/modules)
#add_subdirectory(src/acs)
add_subdirectory(src/epg)
add_subdirectory(src/porting)
add_subdirectory(src/apps)
add_subdirectory(src/3rdparty)

