project(gui_test C CXX)

aux_source_directory(./ SRCS_GUI_TESTS)

include_directories(
   ${CMAKE_SOURCE_DIR}/include
   ${CMAKE_SOURCE_DIR}/include/gui
   ${CMAKE_SOURCE_DIR}/include/porting
   ${CMAKE_SOURCE_DIR}/include/cairo
   ${CMAKE_SOURCE_DIR}/src/3rdparty/cairo-1.16.0/src
   ${CMAKE_SOURCE_DIR}/include/epg
)

add_executable(gui_test ${SRCS_GUI_TESTS})

target_link_libraries(gui_test gui gtest dvbepg)

install(TARGETS gui_test DESTINATION bin)

