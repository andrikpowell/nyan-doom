cmake_minimum_required(VERSION 3.9)

option(WITH_FLUIDSYNTH "Use FluidSynth if available" ON)
if(WITH_FLUIDSYNTH)
    list(APPEND VCPKG_MANIFEST_FEATURES "fluidsynth")
endif()

option(WITH_IMAGE "Use SDL2_image if available" ON)
if(WITH_IMAGE)
    list(APPEND VCPKG_MANIFEST_FEATURES "sdl2-image")
endif()

option(WITH_MAD "Use libmad if available" ON)
if(WITH_MAD)
    list(APPEND VCPKG_MANIFEST_FEATURES "libmad")
endif()

option(WITH_PORTMIDI "Use PortMidi if available" ON)
if(WITH_PORTMIDI)
    list(APPEND VCPKG_MANIFEST_FEATURES "portmidi")
endif()

option(WITH_VORBISFILE "Use vorbisfile if available" ON)
if(WITH_VORBISFILE)
    list(APPEND VCPKG_MANIFEST_FEATURES "libvorbis")
endif()

option(WITH_LIBOPENMPT "Use libopenmpt if available" ON)
if(WITH_LIBOPENMPT)
    list(APPEND VCPKG_MANIFEST_FEATURES "libopenmpt")
endif()

# Automatically install dependencies
set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)
set(X_VCPKG_APPLOCAL_DEPS_INSTALL TRUE)

project("nyan-doom" VERSION 1.3.5)

# Set a default build type if none was specified
set(default_build_type "RelWithDebInfo")
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "Setting build type to '${default_build_type}' as none was specified.")
  set(CMAKE_BUILD_TYPE "${default_build_type}" CACHE
      STRING "Choose the type of build." FORCE)
  # Set the possible values of build type for cmake-gui
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
    "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include(GNUInstallDirs)

if(POLICY CMP0099)
  cmake_policy(SET CMP0099 NEW)
else()
  message(WARNING "Your version of CMake is very old. This may cause linking issues if your dependencies are not in your compiler's default search paths.")
endif()

option(CMAKE_FIND_PACKAGE_PREFER_CONFIG "Search for package config before using Find modules" ON)

if(VCPKG_TOOLCHAIN)
    set(ENV{PKG_CONFIG_PATH} "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/lib/pkgconfig")
endif()

if(POLICY CMP0069)
    cmake_policy(SET CMP0069 NEW)
endif()

include(CheckIPOSupported)
check_ipo_supported(RESULT HAVE_LTO)

include(CMakeDependentOption)
cmake_dependent_option(ENABLE_LTO "Use link-time optimisation" OFF "HAVE_LTO" OFF)

set(PROJECT_TARNAME "nyan-doom")
set(WAD_DATA "nyan-doom.wad")
set(PROJECT_STRING "${PROJECT_NAME} ${PROJECT_VERSION}")

include(NyanTargetFeatures)

include(PkgConfigHelper)

set(OpenGL_GL_PREFERENCE LEGACY)
find_package(OpenGL 2.0 REQUIRED)
find_package(libzip REQUIRED)
find_package(SDL2 2.0.7 REQUIRED)
find_package(SDL2_mixer REQUIRED)
find_package(ZLIB REQUIRED)

if(NOT TARGET SDL2_mixer::SDL2_mixer)
    add_library(SDL2_mixer::SDL2_mixer ALIAS SDL2_mixer::SDL2_mixer-static)
endif()

if(WITH_IMAGE)
    find_package(SDL2_image)
    if(SDL2_image_FOUND)
        set(HAVE_LIBSDL2_IMAGE TRUE)
        if(NOT TARGET SDL2_image::SDL2_image)
            add_library(SDL2_image::SDL2_image ALIAS SDL2_image::SDL2_image-static)
        endif()
    endif()
endif()

if(WITH_MAD)
    find_package(LibMad)
    if(LibMad_FOUND)
        set(HAVE_LIBMAD TRUE)
    endif()
endif()

if(WITH_FLUIDSYNTH)
    find_package(FluidSynth)
    if(FluidSynth_FOUND)
        set(HAVE_LIBFLUIDSYNTH TRUE)
    endif()
endif()

if(WITH_VORBISFILE)
    find_package(Vorbis COMPONENTS File)
    if(Vorbis_File_FOUND)
        set(HAVE_LIBVORBISFILE TRUE)
    endif()
endif()

if(WITH_PORTMIDI)
    find_package(PortMidi)
    if(PortMidi_FOUND)
        set(HAVE_LIBPORTMIDI TRUE)
    endif()
endif()

if(WITH_LIBOPENMPT)
    find_package(libopenmpt)
    if(libopenmpt_FOUND)
        set(HAVE_LIBOPENMPT TRUE)
    endif()
endif()

if(WIN32)
    set(DEFAULT_WAD_DIR ".")
else()
    set(DEFAULT_WAD_DIR "share/games/doom")
endif()

set(NYANPWADDIR "${DEFAULT_WAD_DIR}" CACHE STRING "Path to install Nyan-Doom internal WAD, relative to CMAKE_INSTALL_PREFIX or absolute.")
set(DOOMWADDIR "${CMAKE_INSTALL_PREFIX}/${DEFAULT_WAD_DIR}" CACHE PATH "Path to look for WAD files.")

if(IS_ABSOLUTE "${NYANPWADDIR}")
    set(NYAN_ABSOLUTE_PWAD_PATH "${NYANPWADDIR}")
else()
    set(NYAN_ABSOLUTE_PWAD_PATH "${CMAKE_INSTALL_PREFIX}/${NYANPWADDIR}")
endif()

option(SIMPLECHECKS "Enable checks which only impose significant overhead if a posible error is detected" ON)

# Debug options, disabled by default
option(RANGECHECK "Enable internal range checking" OFF)

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

if(WIN32)
    install(FILES COPYING DESTINATION . RENAME COPYING.txt)
else()
    install(FILES COPYING DESTINATION "share/doc/${PROJECT_NAME}")
endif()

if(WIN32)
    set(CPACK_GENERATOR ZIP)
elseif(LINUX)
    set(CPACK_GENERATOR External)
    set(CPACK_EXTERNAL_ENABLE_STAGING YES)
    set(CPACK_PACKAGING_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")
    set(CPACK_EXTERNAL_PACKAGE_SCRIPT "${PROJECT_BINARY_DIR}/appimage-generate.cmake")

    file(GENERATE
        OUTPUT "${PROJECT_BINARY_DIR}/appimage-generate.cmake"
        CONTENT [[
            find_program(LINUXDEPLOY_EXECUTABLE
                NAMES linuxdeploy linuxdeploy-x86_64.AppImage
                PATHS ${CPACK_PACKAGE_DIRECTORY}/linuxdeploy)

            if(NOT LINUXDEPLOY_EXECUTABLE)
                message(STATUS "Downloading linuxdeploy")
                set(LINUXDEPLOY_EXECUTABLE ${CPACK_PACKAGE_DIRECTORY}/linuxdeploy/linuxdeploy)
                file(DOWNLOAD
                    https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage
                    ${LINUXDEPLOY_EXECUTABLE}
                    INACTIVITY_TIMEOUT 10)
                execute_process(COMMAND
                    chmod +x ${LINUXDEPLOY_EXECUTABLE} COMMAND_ECHO STDOUT)
            endif()

            execute_process(COMMAND
                ${CMAKE_COMMAND} -E env
                OUTPUT=${CPACK_PACKAGE_FILE_NAME}.appimage
                VERSION=$<IF:$<BOOL:${CPACK_PACKAGE_VERSION}>,${CPACK_PACKAGE_VERSION},0.1.0>
                ${LINUXDEPLOY_EXECUTABLE}
                --appimage-extract-and-run
                --appdir=${CPACK_TEMPORARY_DIRECTORY}
                --executable=$<TARGET_FILE:nyan-doom>
                --desktop-file=${CPACK_TEMPORARY_DIRECTORY}/${CPACK_PACKAGING_INSTALL_PREFIX}/share/applications/nyan-doom.desktop
                --icon-file=${CPACK_TEMPORARY_DIRECTORY}/${CPACK_PACKAGING_INSTALL_PREFIX}/share/icons/hicolor/scalable/apps/nyan-doom.svg
                --output=appimage)
            ]])
elseif(APPLE)
    set(CPACK_GENERATOR External)
    set(CPACK_EXTERNAL_ENABLE_STAGING YES)
    set(CPACK_PACKAGING_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")
    set(CPACK_EXTERNAL_PACKAGE_SCRIPT "${PROJECT_BINARY_DIR}/macbundle-generate.cmake")
    set(CPACK_SYSTEM_NAME "mac-${CMAKE_SYSTEM_PROCESSOR}")
    set(CPACK_SYSTEM_PROCESSOR "${CMAKE_SYSTEM_PROCESSOR}")
    set(CPACK_BUILD_DIR "${PROJECT_BINARY_DIR}")

    file(GENERATE
        OUTPUT "${PROJECT_BINARY_DIR}/macbundle-generate.cmake"
        CONTENT [[
            set(CPACK_OUTPUT_DIR "${CPACK_BUILD_DIR}/${CPACK_PACKAGE_FILE_NAME}")
            file(MAKE_DIRECTORY ${CPACK_OUTPUT_DIR})

            find_program(DYLIBBUNDLER_EXECUTABLE NAMES dylibbundler)
            if(NOT DYLIBBUNDLER_EXECUTABLE)
                message(FATAL_ERROR "Missing dylibbundler (brew install dylibbundler)")
            endif()

            file(COPY_FILE
                $<TARGET_FILE:nyan-doom>
                ${CPACK_OUTPUT_DIR}/nyan-doom)

            execute_process(COMMAND
                ${CMAKE_COMMAND} -E env
                OUTPUT=${CPACK_PACKAGE_FILE_NAME}.zip
                VERSION=$<IF:$<BOOL:${CPACK_PACKAGE_VERSION}>,${CPACK_PACKAGE_VERSION},0.1.0>
                ${DYLIBBUNDLER_EXECUTABLE}
                --bundle-deps
                --create-dir
                --fix-file ${CPACK_OUTPUT_DIR}/nyan-doom
                --install-path @executable_path/libs_${CPACK_SYSTEM_PROCESSOR}
                --dest-dir ${CPACK_OUTPUT_DIR}/libs_${CPACK_SYSTEM_PROCESSOR})

            file(COPY_FILE
                ${CPACK_TEMPORARY_DIRECTORY}/${CPACK_PACKAGING_INSTALL_PREFIX}/share/games/doom/nyan-doom.wad
                ${CPACK_OUTPUT_DIR}/nyan-doom.wad)
            file(COPY_FILE
                ${CPACK_TEMPORARY_DIRECTORY}/${CPACK_PACKAGING_INSTALL_PREFIX}/share/doc/nyan-doom/COPYING
                ${CPACK_OUTPUT_DIR}/COPYING.txt)

            file(CONFIGURE
                OUTPUT ${CPACK_OUTPUT_DIR}/Troubleshooting.txt
                CONTENT "If you are getting errors like 'libzip.5.5.dylib cant be opened because Apple cannot check it for malicious software.' Run the following command in the nyan-doom folder:\n\nxattr -dr com.apple.quarantine path/to/folder")

            execute_process(COMMAND zip -r ${CPACK_PACKAGE_FILE_NAME}.zip ${CPACK_PACKAGE_FILE_NAME})
            ]])
endif()
include(CPack)
