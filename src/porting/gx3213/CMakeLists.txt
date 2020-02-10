
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
   ngl_snd.c
   ngl_disp.c
   ngl_disp.c
   ngl_pvr.c
   ngl_ir.cc
   ngl_panel.c
   ngl_mediaplayer.c
)

include(platform.cmake)

find_package(GTK3)
find_package(X11)
if ( RFB_GRAPH )
elseif ( X11_FOUND )
   set(SRC_GRAPH ngl_x11_graph.c)
elseif ( GTK3_FOUND )
   set(SRC_GRAPH ngl_gtk_graph.c)
else()
   set(SRC_GRAPH ngl_fb_graph.c)
endif()

add_library(x86 STATIC ${SRCS_X86} ${SRC_GRAPH})
if ( RFB_GRAPH )
   message("RFB_GRAPH is used")
elseif (SRC_GRAPH)
   message("SRC_GRAPH=${SRC_GRAPH}")
else()
   message(FATAL_ERROR "no graph module is selected,please check your enviroment!")
endif()

include_directories(
	./ 
	../inc 
	${HAL_EXTRA_INCDIRS})

install (TARGETS x86  DESTINATION lib)
