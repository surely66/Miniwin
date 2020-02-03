#cmake_minimum_required (VERSION 3.8)
project(haltest C CXX)

aux_source_directory(./ SRCS_HAL_TESTS)
include_directories(
   ./
   ${CMAKE_SOURCE_DIR}/include
   ${CMAKE_SOURCE_DIR}/include/porting
   ${CMAKE_BINARY_DIR}/include/
   ${CMAKE_BINARY_DIR}/include/pixman-1
   ${CMAKE_BINARY_DIR}/include/freetype2
)
add_executable(hal_tests ${SRCS_HAL_TESTS}) 

target_link_libraries(hal_tests  tvhal pixman-1 png freetype gtest)

install (TARGETS hal_tests DESTINATION bin)
