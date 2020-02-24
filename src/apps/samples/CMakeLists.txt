cmake_minimum_required (VERSION 2.6)
project(gui_test C CXX)

set(CMAKE_CXX_STANDARD 11)

include_directories(
   ${CMAKE_BINARY_DIR}/include
   ${CMAKE_BINARY_DIR}/include/gui
   ${CMAKE_BINARY_DIR}/include/porting
   ${CMAKE_BINARY_DIR}/include/cairo
   ${CMAKE_BINARY_DIR}/include/epg
)
set(CMAKE_CXX_FLAGS "-Wl,--copy-dt-needed-entries")  #prevent error adding symbols: DSO missing from command line
add_executable(helloworld helloworld.cc)
add_executable(simplewin simplewin.cc)
add_executable(multiwin multiwindow.cc)
add_executable(2048 game2048.cc)

target_link_libraries(helloworld  ${NGL_CHIPSET} ${HAL_EXTRA_LIBS} tvhal_static gui_static ${GUI_DEP_LIBS})
target_link_libraries(simplewin gui)
target_link_libraries(multiwin gui)
target_link_libraries(2048 gui)

install (TARGETS helloworld  DESTINATION bin)
install (TARGETS simplewin DESTINATION bin)
install (TARGETS multiwin DESTINATION bin)
install (TARGETS 2048 DESTINATION bin)
