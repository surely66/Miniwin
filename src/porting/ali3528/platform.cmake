#set platform include dirs & library path

set(NGL_PLATFORM_INCDIRS
 ${TOOLCHAIN_DIR}/target/mipsel-buildroot-linux-gnu/sysroot/usr/include/directfb 
 ${TOOLCHAIN_DIR}/host/usr/mipsel-buildroot-linux-gnu/sysroot/usr/include/directfb
)
set(NGL_PLATFORM_LIBDIRS
 ${TOOLCHAIN_DIR}/target/mipsel-buildroot-linux-gnu/sysroot/usr/lib
 ${TOOLCHAIN_DIR}/target/mipsel-buildroot-linux-gnu/sysroot/lib
)

set(NGL_PLATFORM_LIBS pthread aui direct directfb)
