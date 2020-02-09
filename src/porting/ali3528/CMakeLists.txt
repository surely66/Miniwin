
project(ali3528 C CXX)


set(SRCS_3528 
   ngl_os.c
   ngl_msgq.c
   ngl_dmx.cc
   ngl_timer.c
   ngl_nvm.c
   ngl_tuner.c
   ngl_smc.c
   ngl_video.c
   ngl_snd.c
   ngl_disp.c
   ngl_panel.c
   ngl_pvr.c
   ngl_ir.cc
   ngl_mediaplayer.c
#   ngl_descramble.c
   ngl_dsc.c
)
if( NOT RFB_GRAPH )
   set(ALI_GRAPH_SRC ngl_graph.c)
else()
   message("Notice :${NGL_CHIPSET} used RFB(Remote FrameBuffer),it's only used for test")
endif()

add_library(ali3528 STATIC ${SRCS_3528} ${ALI_GRAPH_SRC})
include(platform.cmake)

message("DIRECTFB_INCLUDE_DIR=${DIRECTFB_INCLUDE_DIR}")

include_directories(${HAL_EXTRA_INCLUDE_DIR} ${HAL_EXTRA_INCDIRS}
  ../inc ./)

#target_link_libraries(alitest rt direct directfb pthread aui)


install (TARGETS ali3528  DESTINATION lib)
