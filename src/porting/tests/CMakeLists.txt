#cmake_minimum_required (VERSION 3.8)
project(haltest C CXX)

aux_source_directory(./ SRCS_HAL_TESTS)
include_directories(
   ${CMAKE_SOURCE_DIR}/include
   ${CMAKE_SOURCE_DIR}/include/porting
)
add_executable(hal_tests ${SRCS_HAL_TESTS}) 

target_link_libraries(hal_tests  tvhal gtest)

install (TARGETS hal_tests DESTINATION bin)
