#----------------------------------------------------------------
# Generated CMake target import file.
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "iconv" for configuration ""
set_property(TARGET iconv APPEND PROPERTY IMPORTED_CONFIGURATIONS NOCONFIG)
set_target_properties(iconv PROPERTIES
  IMPORTED_LINK_INTERFACE_LIBRARIES_NOCONFIG "charset"
  IMPORTED_LOCATION_NOCONFIG "${_IMPORT_PREFIX}/lib/libiconv.so"
  IMPORTED_SONAME_NOCONFIG "libiconv.so"
  )

list(APPEND _IMPORT_CHECK_TARGETS iconv )
list(APPEND _IMPORT_CHECK_FILES_FOR_iconv "${_IMPORT_PREFIX}/lib/libiconv.so" )

# Import target "charset" for configuration ""
set_property(TARGET charset APPEND PROPERTY IMPORTED_CONFIGURATIONS NOCONFIG)
set_target_properties(charset PROPERTIES
  IMPORTED_LOCATION_NOCONFIG "${_IMPORT_PREFIX}/lib/libcharset.so"
  IMPORTED_SONAME_NOCONFIG "libcharset.so"
  )

list(APPEND _IMPORT_CHECK_TARGETS charset )
list(APPEND _IMPORT_CHECK_FILES_FOR_charset "${_IMPORT_PREFIX}/lib/libcharset.so" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
