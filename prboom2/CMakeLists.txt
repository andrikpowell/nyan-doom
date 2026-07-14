cmake_minimum_required(VERSION 3.17...4.0)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

# Setup dependencies options before `project()` so vcpkg features can be used
include(PenguinoDepsSetup)

project("penguino-doom" VERSION 1.0.0)

set(penguino_is_top_project FALSE)
if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    set(penguino_is_top_project TRUE)
endif()

if(penguino_is_top_project)
    include(PenguinoSanitiser)
endif()

include(PenguinoHelpers)
if(penguino_is_top_project)
    penguino_set_default_build_config(RelWithDebInfo)
endif()

if(VCPKG_TOOLCHAIN)
    set(ENV{PKG_CONFIG_PATH} "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/lib/pkgconfig")
endif()

set(PROJECT_TARNAME "penguino-doom")
set(WAD_DATA "penguino-doom.wad")

set(PROJECT_STRING "${PROJECT_NAME} v${PROJECT_VERSION}")
set(PROJECT_DEMO_STRING "${PROJECT_NAME} ${PROJECT_VERSION}")

include(PenguinoOptions)

if(Penguino_NIGHTLY)
  string(APPEND PROJECT_STRING " (nightly)")
  string(APPEND PROJECT_DEMO_STRING " (nightly)")
endif()

include(PenguinoTargetFeatures)
include(PenguinoDependencies)
include(PenguinoConfigHeader)

set(PENGUINO_OUTPUT_PATH ${CMAKE_BINARY_DIR})

set(WAD_DATA_PATH "${PENGUINO_OUTPUT_PATH}/${WAD_DATA}")

set(PENGUINO_BUILD_PWAD_WITH_EXTERNAL_PROJECT ${CMAKE_CROSSCOMPILING})
if(CMAKE_GENERATOR MATCHES "Visual Studio")
    set(PENGUINO_BUILD_PWAD_WITH_EXTERNAL_PROJECT TRUE)
    set(PENGUINO_EXTERNAL_PROJECT_VS_ARGS
        "CMAKE_GENERATOR" "${CMAKE_GENERATOR}"
        "CMAKE_GENERATOR_PLATFORM" "${CMAKE_VS_PLATFORM_NAME_DEFAULT}"
    )
endif()

if(PENGUINO_BUILD_PWAD_WITH_EXTERNAL_PROJECT)
    set(PENGUINO_HOST_TOOLCHAIN ""
        CACHE FILEPATH "Toolchain file to build host tools with"
    )
    include(ExternalProject)
    ExternalProject_Add(penguino-doom-wad
        SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/data"
        ${PENGUINO_EXTERNAL_PROJECT_VS_ARGS}
        CMAKE_CACHE_ARGS
            "-DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}"
            "-DWAD_DATA_PATH:FILEPATH=${WAD_DATA_PATH}"
            "-DCMAKE_TOOLCHAIN_FILE:FILEPATH=${PENGUINO_HOST_TOOLCHAIN}"
            "-DCMAKE_C_FLAGS:STRING=$ENV{CFLAGS_FOR_BUILD} $ENV{CPPFLAGS_FOR_BUILD}"
            "-DCMAKE_EXE_LINKER_FLAGS:STRING=$ENV{LDFLAGS_FOR_BUILD}"
        BUILD_ALWAYS TRUE
        BUILD_BYPRODUCTS "${WAD_DATA_PATH}"
        INSTALL_COMMAND ""
    )
else()
    add_subdirectory(data)
endif()

add_subdirectory(src)
add_subdirectory(ICONS)

install(FILES
    "${WAD_DATA_PATH}"
    DESTINATION "${PENGUINOPWADDIR}"
)

install(FILES
    "COPYING"
    DESTINATION "${PENGUINO_INSTALL_COPYRIGHT_DIR}"
)

if(ENABLE_PACKAGING)
    add_subdirectory(packaging)
endif()
