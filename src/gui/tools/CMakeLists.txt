project(respack C CXX)

set(CMAKE_CROSSCOMPILING false)
set(CMAKE_CXX_COMPILER g++)
set(CMAKE_C_COMPILER gcc)
set(SRCS_PAK ${CMAKE_SOURCE_DIR}/src/gui/core/filepak.cc
             ${CMAKE_SOURCE_DIR}/src/gui/core/gzstream.cc
             ./main.cc
)
include_directories(  ../core/
  ${CMAKE_SOURCE_DIR}/src/3rdparty/jsoncpp/include
  ./
)

find_package(z)

add_executable(respack ${SRCS_PAK})

target_link_libraries(respack z)

link_directories(${CMAKE_PROJECT_DIR}/libs)
#add_executable(lansplit lansplit.cc)
#target_link_libraries(lansplit jsoncpp)


add_executable(tssender tsserver.cc tssender.cc tsreceiver.cc)
add_executable(tsreceiver tsreceiver.cc tssender.cc)
#target_compile_definitions(tssender PUBLIC BUILD_SND_DEMO=1)
target_compile_definitions(tsreceiver PUBLIC BUILD_RCV_DEMO=1)
target_link_libraries(tssender pthread)

add_custom_command (TARGET respack POST_BUILD
#  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/resource.h   #输出文件
  COMMAND cp ${CMAKE_CURRENT_BINARY_DIR}/respack  ${CMAKE_SOURCE_DIR}
  WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
  DEPENDS respack
  COMMENT "cp respack "
  )

install(TARGETS respack DESTINATION bin)

