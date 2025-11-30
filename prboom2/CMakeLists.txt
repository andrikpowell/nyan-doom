cmake_minimum_required(VERSION 3.17...4.0)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

# Setup dependencies options before `project()` so vcpkg features can be used
include(NyanDepsSetup)

project("nyan-doom" VERSION 1.4.1)

set(nyan_is_top_project FALSE)
if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    set(nyan_is_top_project TRUE)
endif()

if(nyan_is_top_project)
    include(NyanSanitiser)
endif()

include(NyanHelpers)
if(nyan_is_top_project)
    nyan_set_default_build_config(RelWithDebInfo)
endif()

if(VCPKG_TOOLCHAIN)
    set(ENV{PKG_CONFIG_PATH} "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/lib/pkgconfig")
endif()

set(PROJECT_TARNAME "nyan-doom")
set(WAD_DATA "nyan-doom.wad")
set(PROJECT_STRING "${PROJECT_NAME} ${PROJECT_VERSION}")

include(NyanOptions)
include(NyanTargetFeatures)
include(NyanDependencies)
include(NyanConfigHeader)

set(NYAN_OUTPUT_PATH ${CMAKE_BINARY_DIR})

set(WAD_DATA_PATH "${NYAN_OUTPUT_PATH}/${WAD_DATA}")

set(NYAN_BUILD_PWAD_WITH_EXTERNAL_PROJECT ${CMAKE_CROSSCOMPILING})
if(CMAKE_GENERATOR MATCHES "Visual Studio")
    set(NYAN_BUILD_PWAD_WITH_EXTERNAL_PROJECT TRUE)
    set(NYAN_EXTERNAL_PROJECT_VS_ARGS
        "CMAKE_GENERATOR" "${CMAKE_GENERATOR}"
        "CMAKE_GENERATOR_PLATFORM" "${CMAKE_VS_PLATFORM_NAME_DEFAULT}"
    )
endif()

if(NYAN_BUILD_PWAD_WITH_EXTERNAL_PROJECT)
    set(NYAN_HOST_TOOLCHAIN ""
        CACHE FILEPATH "Toolchain file to build host tools with"
    )
    include(ExternalProject)
    ExternalProject_Add(nyan-doom-wad
        SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/data"
        ${NYAN_EXTERNAL_PROJECT_VS_ARGS}
        CMAKE_CACHE_ARGS
            "-DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}"
            "-DWAD_DATA_PATH:FILEPATH=${WAD_DATA_PATH}"
            "-DCMAKE_TOOLCHAIN_FILE:FILEPATH=${NYAN_HOST_TOOLCHAIN}"
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
    DESTINATION "${NYANPWADDIR}"
)

install(FILES
    "COPYING"
    DESTINATION "${NYAN_INSTALL_COPYRIGHT_DIR}"
)

if(ENABLE_PACKAGING)
    add_subdirectory(packaging)
endif()
