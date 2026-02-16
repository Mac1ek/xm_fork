#----------------------------------------------------------------
# Generated CMake target import file.
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "xmplayer::xmplayer" for configuration ""
set_property(TARGET xmplayer::xmplayer APPEND PROPERTY IMPORTED_CONFIGURATIONS NOCONFIG)
set_target_properties(xmplayer::xmplayer PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_NOCONFIG "CXX"
  IMPORTED_LOCATION_NOCONFIG "${_IMPORT_PREFIX}/lib/libxmplayer.a"
  )

list(APPEND _cmake_import_check_targets xmplayer::xmplayer )
list(APPEND _cmake_import_check_files_for_xmplayer::xmplayer "${_IMPORT_PREFIX}/lib/libxmplayer.a" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
