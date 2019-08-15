
project(ali3528 C CXX)


set(SRCS_3528 
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

add_library(ali3528 STATIC ${SRCS_3528})
include(platform.cmake)

message("DIRECTFB_INCLUDE_DIR=${DIRECTFB_INCLUDE_DIR}")

include_directories(${DIRECTFB_INCLUDE_DIR} ${NGL_PLATFORM_INCDIRS}
  ../inc ./)

#target_link_libraries(alitest rt direct directfb pthread aui)


install (TARGETS ali3528  DESTINATION lib)
