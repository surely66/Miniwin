project(ntvplus C CXX)

aux_source_directory(./ SRCS_NTV_PLUS)

include_directories(
   ./
   ${CMAKE_SOURCE_DIR}/include
   ${CMAKE_SOURCE_DIR}/include/gui
   ${CMAKE_SOURCE_DIR}/include/porting
   ${CMAKE_SOURCE_DIR}/include/cairo
   ${CMAKE_SOURCE_DIR}/include/epg
)


add_executable(ntvplus ${SRCS_NTV_PLUS} ntvplus.pak)
target_link_libraries(ntvplus gui dvbepg )

message("CMAKE_CURRENT_BINARY_DIR=${CMAKE_CURRENT_BINARY_DIR}")

add_custom_command (TARGET ntvplus PRE_BUILD
#  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/resource.h   #输出文件
  COMMAND ${CMAKE_SOURCE_DIR}/respack ntvplus.pak assets/
  COMMAND cp ntvplus.pak ${CMAKE_CURRENT_BINARY_DIR}/
  BYPRODUCTS ntvplus
  WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
  DEPENDS respack
  COMMENT "package assets resource"
  )

install (TARGETS ntvplus  DESTINATION bin)
install (FILES ntvplus.pak  DESTINATION bin)
