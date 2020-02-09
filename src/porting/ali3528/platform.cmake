#set platform include dirs & library path

set(HAL_EXTRA_INCDIRS
 ${TOOLCHAIN_DIR}/target/mipsel-buildroot-linux-gnu/sysroot/usr/include/directfb 
 ${TOOLCHAIN_DIR}/host/usr/mipsel-buildroot-linux-gnu/sysroot/usr/include/directfb
)
set(HAL_EXTRA_LIBDIRS
 ${TOOLCHAIN_DIR}/target/mipsel-buildroot-linux-gnu/sysroot/usr/lib
 ${TOOLCHAIN_DIR}/target/mipsel-buildroot-linux-gnu/sysroot/lib
)

set(HAL_EXTRA_LIBS pthread aui direct directfb)
