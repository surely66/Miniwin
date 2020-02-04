
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
   ngl_mediaplayer.c
)

include(platform.cmake)

find_package(GTK3)
find_package(X11)
if (TRUE)
   set(SRC_GRAPH ngl_vnc_graph.c)
elseif ( X11_FOUND )
   set(SRC_GRAPH ngl_x11_graph.c)
elseif ( GTK3_FOUND )
   set(SRC_GRAPH ngl_gtk_graph.c)
else()
   set(SRC_GRAPH ngl_fb_graph.c)
endif()

add_library(nglx86 STATIC ${SRCS_X86} ${SRC_GRAPH})

message("==SRC_GRAPH=${SRC_GRAPH}")
message("NGL_PLATFORM_INCDIRS=${NGL_PLATFORM_INCDIRS}")
message("NGL_PLATFORM_LIBS=${NGL_PLATFORM_LIBS}")

include_directories(${NGL_PLATFORM_INCDIRS}
  ../inc ./)


install (TARGETS nglx86  DESTINATION lib)
