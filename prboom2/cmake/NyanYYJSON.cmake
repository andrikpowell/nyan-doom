find_package(yyjson CONFIG QUIET)
if(yyjson_FOUND)
  return()
endif()

include(FetchContent)

set(YYJSON_BUILD_TESTS OFF CACHE BOOL "Build yyjson tests" FORCE)
set(YYJSON_BUILD_FUZZER OFF CACHE BOOL "Build yyjson fuzzer" FORCE)
set(YYJSON_BUILD_MISC OFF CACHE BOOL "Build yyjson misc tools" FORCE)
set(YYJSON_BUILD_DOC OFF CACHE BOOL "Build yyjson documentation" FORCE)
set(YYJSON_INSTALL OFF CACHE BOOL "Generate yyjson install target" FORCE)

FetchContent_Declare(yyjson
  GIT_REPOSITORY https://github.com/ibireme/yyjson.git
  GIT_TAG 0.12.0
)
FetchContent_MakeAvailable(yyjson)

if(TARGET yyjson AND NOT TARGET yyjson::yyjson)
  add_library(yyjson::yyjson ALIAS yyjson)
endif()

set(yyjson_FOUND TRUE)
