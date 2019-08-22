
project(acs  C CXX)

set(SRCS_ACS
   src/va_ctrl.c  src/va_fs.c    src/va_net.c   src/va_pvr.c    src/va_vse.c
   src/va_dmx.c   src/va_init.c  src/va_os.c    src/va_sc.c     src/va_xnvm.c
   src/va_prot.c  src/va_http.c
   src/va_dscr.c   src/va_schip.c
   src/va_crypt.c 
#  Stubs/Src/va_dscr.c   Stubs/Src/va_schip.c
)
list(REMOVE_ITEM SRCS_ACS ./Stubs/Src/stub_main.c ./Stubs/Src/dmxtest.c)
message("SRCS_ACS=${SRCS_ACS}")


add_library(acs_static STATIC ${SRCS_ACS})
add_library(acs SHARED ${SRCS_ACS})

add_definitions(-DVAOPT_ENABLE_SCHIP=1)
include_directories(
     include
     ../../include/
     ../porting/inc/
     ${CMAKE_BINARY_DIR}/include
     ${CMAKE_SOURCE_DIR}/include/curl
     #${TOOLCHAIN_DIR}/host/usr/mipsel-buildroot-linux-gnu/sysroot/usr/include/aliplatform
     #${TOOLCHAIN_DIR}/host/usr/mipsel-buildroot-linux-gnu/sysroot/usr/include/ali_common
)
target_link_libraries(acs pthread aui)

link_directories(./ ./Project/ACS/Interface )
add_executable(acstest main.c )
#stub_main.c )
target_link_libraries(acstest pthread aui tvhal curl acs acs_va m )

add_executable(sctest sctest.c stub_main.c)
target_link_libraries(sctest aui pthread acs_va tvhal acs curl m)

install (TARGETS acs  DESTINATION lib)
install (TARGETS acs_static  DESTINATION lib)
install (TARGETS acstest  DESTINATION bin)

install (TARGETS sctest  DESTINATION bin)
