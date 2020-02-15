project(ntvplus C CXX)

aux_source_directory(./ SRCS_NTV_PLUS)

include_directories(
   ./
   ./acs/include
   ${CMAKE_BINARY_DIR}/include
   ${CMAKE_BINARY_DIR}/include/gui
   ${CMAKE_BINARY_DIR}/include/porting
   ${CMAKE_BINARY_DIR}/include/cairo
   ${CMAKE_BINARY_DIR}/include/epg
)

if (NGL_CA STREQUAL "acs")
    add_subdirectory(acs)
    link_directories(acs)
endif()
add_executable(ntvplus ${SRCS_NTV_PLUS} ntvplus.pak)
if(CMAKE_SYSTEM_PROCESSOR MATCHES "x86|i686|amd|AMD" )
target_link_libraries(ntvplus jsoncpp_lib gui dvbepg )
else()
target_link_libraries(ntvplus jsoncpp_lib gui dvbepg acs)
endif()

message("CMAKE_CURRENT_BINARY_DIR=${CMAKE_CURRENT_BINARY_DIR}")

add_custom_command (TARGET ntvplus POST_BUILD
  COMMAND ${CMAKE_SOURCE_DIR}/respack assets ${CMAKE_CURRENT_BINARY_DIR}/ntvplus.pak
  BYPRODUCTS ntvplus
  WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
  DEPENDS respack
  COMMENT "package assets resource"
  )

install (TARGETS ntvplus  DESTINATION bin)
install (FILES ntvplus.pak  DESTINATION bin)
