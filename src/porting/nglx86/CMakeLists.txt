
project(nglx86 C CXX)


set(SRCS_X86 
   ngl_os.c
   ngl_msgq.c
   ngl_dmx.c
   ngl_timer.c
   ngl_dsc.c
   ngl_nvm.c
   ngl_tuner.c
   ngl_smc.c
   ngl_video.c
   ngl_audio.c
   ngl_disp.c
   ngl_pvr.c
   ngl_ir.cc
   ngl_graph.c
   ngl_mediaplayer.c
)

add_library(nglx86 STATIC ${SRCS_X86})
include(platform.cmake)

find_package(GTK2  REQUIRED gtk)

include_directories(${GTK2_INCLUDE_DIRS}
  ../inc ./)


install (TARGETS nglx86  DESTINATION lib)
