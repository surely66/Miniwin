SET(CMAKE_SYSTEM_NAME eCos)

#SET(TOOLCHAIN_DIR $ENV{HOME}/toolchain_montage/mips-4.3)
set(TOOLCHAIN_DIR /opt/montage/mips-4.3/)

set(CMAKE_C_COMPILER_FORCED TRUE)
set(CMAKE_CXX_COMPILER_FORCED TRUE)
set(CMAKE_CROSSCOMPILING true)
set(ECOS_SYSTEM_CONFIG_HEADER_PATH ${TOOLCHAIN_DIR})
set(ECOS_SYSTEM_TARGET_LIBRARY ${TOOLCHAIN_DIR})
SET(CMAKE_CXX_COMPILER ${TOOLCHAIN_DIR}/bin/mips-linux-gnu-g++)
SET(CMAKE_C_COMPILER ${TOOLCHAIN_DIR}/bin/mips-linux-gnu-gcc)
SET(CMAKE_FIND_ROOT_PATH ${TOOLCHAIN_DIR})
#/mips-linux-gnu)
SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
