#cmake_minimum_required (VERSION 3.8)
project(epgtest C CXX)

aux_source_directory(./ SRCS_EPG_TESTS)

add_executable(epgtests ${SRCS_EPG_TESTS}) 

target_link_libraries(epgtests  tvhal dvbepg gtest)

install (TARGETS epgtests DESTINATION bin)
