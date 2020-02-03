#cmake_minimum_required (VERSION 3.8)
project(tvhal C CXX)

if( CMAKE_SYSTEM_NAME MATCHES "Linux" )
add_definitions(-DLINUX=1)
endif()

set(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -O3 -fPIC")
set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -O3 -fPIC")

message("CAPLATFORM=${CA_PLATFORM}")
include(${NGL_CHIP_PLATFORM}/platform.cmake)
message("NGL_CHIP_PLATFORM=${NGL_CHIP_PLATFORM}")
message("NGL_PLATFORM_LIBS=${NGL_PLATFORM_LIBS}")
message("TOOLCHAIN_DIR=${TOOLCHAIN_DIR}")

aux_source_directory(./ SRCS_COMMON)
add_library(tvhal SHARED ${SRCS_COMMON})
add_library(tvhal_static STATIC ${SRCS_COMMON})
include_directories(inc)
add_subdirectory(${NGL_CHIP_PLATFORM})
target_link_libraries(tvhal ${NGL_CHIP_PLATFORM} ${NGL_PLATFORM_LIBS})
target_link_libraries(tvhal_static ${NGL_CHIP_PLATFORM} ${NGL_PLATFORM_LIBS})

add_subdirectory(tests)

add_custom_command(TARGET tvhal POST_BUILD 
    COMMAND mkdir -p ${CMAKE_SOURCE_DIR}/include/porting
    COMMAND cp -u ${PROJECT_SOURCE_DIR}/inc/*.h  ${CMAKE_SOURCE_DIR}/include/porting
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
    COMMENT "Copy NGL Porting Header Files")

file(GLOB HAL_HEADERS  ${PROJECT_SOURCE_DIR}/inc/*.h)

install (TARGETS tvhal  DESTINATION lib)
install (TARGETS tvhal_static  DESTINATION lib)
install (FILES ${HAL_HEADERS} DESTINATION include/porting/)
