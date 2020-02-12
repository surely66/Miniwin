cmake_minimum_required (VERSION 3.2)
project(tvhal C CXX)

if( CMAKE_SYSTEM_NAME MATCHES "Linux" )
add_definitions(-DLINUX=1)
endif()

set(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS}  -fPIC")
set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS}  -fPIC")

include(${NGL_CHIPSET}/platform.cmake)
message("NGL_CHIPSET=${NGL_CHIPSET}  USE_RFB_GRAPH=${USE_RFB_GRAPH}")
message("HAL_EXTRA_LIBS=${HAL_EXTRA_LIBS}")

aux_source_directory(./ SRCS_COMMON)
if( ${USE_RFB_GRAPH} MATCHES "^ON|^YES" )
  set(RFB_GRAPH ON)
  set(HAL_EXTRA_LIBS ${HAL_EXTRA_LIBS} vncserver)
  message(" RFB(remote frame buffer) is selected to ${NGL_CHIPSET}")
else()
  message("RFB is off")
  list(REMOVE_ITEM SRCS_COMMON .//ngl_rfb.c)
endif()
message("SRCCOMMON=${SRCS_COMMON}")
include_directories(inc 
	${CMAKE_BINARY_DIR}/src/3rdparty/libvncserver
	${CMAKE_SOURCE_DIR}/src/3rdparty/libvncserver)

add_library(tvhal SHARED ${SRCS_COMMON})
add_library(tvhal_static STATIC ${SRCS_COMMON})

#set_property(TARGET tvhal PROPERTY POSITION_INDEPENDENT_CODE)

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
