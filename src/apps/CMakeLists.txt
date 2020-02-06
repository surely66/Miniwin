
add_subdirectory(samples)
if(CMAKE_SYSTEM_PROCESSOR MATCHES "x86|i686|amd|AMD" )
	message("pc platform skip ntvplus compile")
else()
add_subdirectory(ntvplus)
endif()
