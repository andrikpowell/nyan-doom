# Variables defined:
#  SPNG_FOUND
#  SPNG_INCLUDE_DIR
#  SPNG_LIBRARY

find_package(SPNG CONFIG QUIET)

if(SPNG_FOUND)
  return()
endif()

find_package(PkgConfig QUIET)
pkg_check_modules(PC_SPNG IMPORTED_TARGET spng)

if(PC_SPNG_FOUND)
  if(NOT TARGET spng::spng)
    add_library(spng::spng ALIAS PkgConfig::PC_SPNG)
  endif()
  set(SPNG_FOUND TRUE)
  set(SPNG_VERSION ${PC_SPNG_VERSION})
  return()
endif()

find_library(SPNG_LIBRARY
  NAMES spng
)

find_path(SPNG_INCLUDE_DIR
  NAMES spng.h
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(SPNG
  REQUIRED_VARS SPNG_LIBRARY SPNG_INCLUDE_DIR
)

if(SPNG_FOUND)
  if(NOT TARGET spng::spng)
    add_library(spng::spng UNKNOWN IMPORTED)
    set_target_properties(spng::spng PROPERTIES
      IMPORTED_LOCATION "${SPNG_LIBRARY}"
      INTERFACE_INCLUDE_DIRECTORIES "${SPNG_INCLUDE_DIR}"
    )
  endif()
endif()

mark_as_advanced(SPNG_LIBRARY SPNG_INCLUDE_DIR)