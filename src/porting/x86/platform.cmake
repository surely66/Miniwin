#set platform include dirs & library path

if ( TRUE )
	set(HAL_EXTRA_LIBS vncserver CACHE INTERNAL "platform dependence libs")
	set(HAL_EXTRA_INCDIRS 
        ${CMAKE_SOURCE_DIR}/src/3rdparty/libvncserver/
        ${CMAKE_BINARY_DIR}/src/3rdparty/libvncserver/
        ${CMAKE_SOURCE_DIR}/src/3rdparty/trfb/src
        CACHE INTERNAL "platform dependence include")
else()
  find_package(GTK3)
  find_package(X11)
  if ( X11_FOUND )
	  set(HAL_EXTRA_LIBS ${X11_LIBRARIES} CACHE INTERNAL "platform dependence libs")
	  set(HAL_EXTRA_INCDIRS ${X11_INCLUDE_DIRS} CACHE INTERNAL "platform dependence include")
  elseif ( GTK3_FOUND )
	  set(HAL_EXTRA_LIBS ${GTK3_LIBRARIES} CACHE INTERNAL "platform dependence libs")
	  set(HAL_EXTRA_INCDIRS ${GTK3_INCLUDE_DIRS} CACHE INTERNAL "platform dependence include")
  endif()
endif()

