find_package(SPNG QUIET)
if(SPNG_FOUND)
  return()
endif()

include(FetchContent)

set(SPNG_SHARED OFF CACHE BOOL "Build shared libspng" FORCE)
set(SPNG_STATIC ON CACHE BOOL "Build static libspng" FORCE)
set(BUILD_EXAMPLES OFF CACHE BOOL "Build libspng examples" FORCE)

FetchContent_Declare(libspng
  GIT_REPOSITORY https://github.com/randy408/libspng.git
  GIT_TAG v0.7.4
)
FetchContent_MakeAvailable(libspng)

if(TARGET spng_static AND NOT TARGET spng::spng_static)
  add_library(spng::spng_static ALIAS spng_static)
endif()

set(SPNG_FOUND TRUE)
