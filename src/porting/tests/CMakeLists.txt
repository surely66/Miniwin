#cmake_minimum_required (VERSION 3.8)
project(haltest C CXX)

aux_source_directory(./ SRCS_HAL_TESTS)
include_directories(
   ./
   ${CMAKE_BINARY_DIR}/include
   ${CMAKE_BINARY_DIR}/include/porting
   ${CMAKE_SOURCE_DIR}/src/3rdparty/libvncserver
   ${CMAKE_BINARY_DIR}/src/3rdparty/libvncserver
   ${CMAKE_BINARY_DIR}/include/pixman
   ${CMAKE_BINARY_DIR}/include/freetype2
)

message("HAL_EXTRA_LIBS=${HAL_EXTRA_LIBS}")

add_executable(hal_tests ${SRCS_HAL_TESTS}) 

target_link_libraries(hal_tests  ${NGL_CHIPSET} ${HAL_EXTRA_LIBS} tvhal_static 
  #pixman-1 png freetype
  gtest)

install (TARGETS hal_tests  DESTINATION bin)
