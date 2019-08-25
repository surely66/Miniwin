#set platform include dirs & library path
find_package(GTK2  REQUIRED gtk)
set(NGL_PLATFORM_INCDIRS ${GTK2_INCLUDE_DIRS} CACHE INTERNAL "platform dependence include")
set(NGL_PLATFORM_LIBDIRS )

set(NGL_PLATFORM_LIBS ${GTK2_LIBRARIES} CACHE INTERNAL "platform dependence libs")
