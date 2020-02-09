cmake_minimum_required (VERSION 3.2)
project(tvhal C CXX)

if( CMAKE_SYSTEM_NAME MATCHES "Linux" )
add_definitions(-DLINUX=1)
endif()

set(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -O3 -fPIC")
set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -O3 -fPIC")

include(${NGL_CHIPSET}/platform.cmake)
message("NGL_CHIPSET=${NGL_CHIPSET}")
message("HAL_EXTRA_LIBS=${HAL_EXTRA_LIBS}")
message("TOOLCHAIN_DIR=${TOOLCHAIN_DIR}")

aux_source_directory(./ SRCS_COMMON)
if( $ENV{USE_RFB_GRAPH} )
  set(RFB_GRAPH ON)
  set(HAL_EXTRA_LIBS ${HAL_EXTRA_LIBS} vncserver)
  message(" RFB(remote frame buffer) is selected to ${NGL_CHIPSET}")
else()
  list(REMOVE_ITEM SRCS_COMMON .//ngl_rfb.c)
endif()
message("SRCCOMMON=${SRCS_COMMON}")
include_directories(inc 
	${CMAKE_BINARY_DIR}/src/3rdparty/libvncserver
	${CMAKE_SOURCE_DIR}/src/3rdparty/libvncserver)

add_library(tvhal SHARED ${SRCS_COMMON})
add_library(tvhal_static STATIC ${SRCS_COMMON})

target_link_libraries(tvhal ${NGL_CHIPSET} ${HAL_EXTRA_LIBS})
target_link_libraries(tvhal_static ${NGL_CHIPSET} ${HAL_EXTRA_LIBS})

add_subdirectory(${NGL_CHIPSET})
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
file(COPY ${HAL_HEADERS} DESTINATION ${CMAKE_SOURCE_DIR}/include/porting)
