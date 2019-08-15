project(respack C CXX)

set(CMAKE_CROSSCOMPILING false)
set(CMAKE_CXX_COMPILER g++)
set(CMAKE_C_COMPILER gcc)
set(SRCS_PAK ${CMAKE_SOURCE_DIR}/src/gui/core/filepak.cc
             ${CMAKE_SOURCE_DIR}/src/gui/core/gzstream.cc
             ./main.cc
)
include_directories(../core/)
find_package(z)
add_executable(respack ${SRCS_PAK})
target_link_libraries(respack z)
add_custom_command (TARGET respack POST_BUILD
#  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/resource.h   #输出文件
  COMMAND cp ${CMAKE_CURRENT_BINARY_DIR}/respack  ${CMAKE_SOURCE_DIR}
  WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
  DEPENDS respack
  COMMENT "cp respack "
  )

install(TARGETS respack DESTINATION bin)

