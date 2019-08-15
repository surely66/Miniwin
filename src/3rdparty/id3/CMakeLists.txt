project(id3 C CXX)

aux_source_directory(. SRCS_ID3)
list( REMOVE_ITEM  SRCS_ID3 ./main.cpp ./mp3file.c)
message("ID3=${SRCS_ID3}")
include_directories(./)
add_library(id3 ${SRCS_ID3})
add_executable(mp3file main.cpp)
target_link_libraries(mp3file id3)

install(TARGETS id3 DESTINATION lib)
install(TARGETS mp3file DESTINATION bin)
