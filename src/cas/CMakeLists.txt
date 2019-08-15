
project(ca C CXX)

aux_source_directory(${CAS_SYSTEM_NAME} SRCS_CA)
add_library(ca ${SRCS_CA})

target_link_libraries(ca porting)

install (TARGETS ca  DESTINATION lib)


