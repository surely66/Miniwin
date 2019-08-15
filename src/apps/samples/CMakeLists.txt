project(gui_test C CXX)

include_directories(
   ${CMAKE_SOURCE_DIR}/include
   ${CMAKE_SOURCE_DIR}/include/gui
   ${CMAKE_SOURCE_DIR}/include/porting
   ${CMAKE_SOURCE_DIR}/include/cairo
   ${CMAKE_SOURCE_DIR}/include/epg
)

add_executable(helloworld helloworld.cc)
add_executable(simplewin simplewin.cc)
add_executable(multiwin multiwindow.cc)

target_link_libraries(helloworld gui)
target_link_libraries(simplewin gui)
target_link_libraries(multiwin gui)

install (TARGETS helloworld  DESTINATION bin)
install (TARGETS simplewin DESTINATION bin)
install (TARGETS multiwin DESTINATION bin)
