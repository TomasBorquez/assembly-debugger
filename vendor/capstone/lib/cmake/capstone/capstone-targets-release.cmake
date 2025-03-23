#----------------------------------------------------------------
# Generated CMake target import file for configuration "Release".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "capstone::capstone_static" for configuration "Release"
set_property(TARGET capstone::capstone_static APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(capstone::capstone_static PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "C"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/libcapstone.a"
  )

list(APPEND _cmake_import_check_targets capstone::capstone_static )
list(APPEND _cmake_import_check_files_for_capstone::capstone_static "${_IMPORT_PREFIX}/lib/libcapstone.a" )

# Import target "capstone::cstool" for configuration "Release"
set_property(TARGET capstone::cstool APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(capstone::cstool PROPERTIES
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/bin/cstool.exe"
  )

list(APPEND _cmake_import_check_targets capstone::cstool )
list(APPEND _cmake_import_check_files_for_capstone::cstool "${_IMPORT_PREFIX}/bin/cstool.exe" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
