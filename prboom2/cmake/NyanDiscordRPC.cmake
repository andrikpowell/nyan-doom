#[==============================================================[
DiscordRPC
-------

DiscordRPC is really old and comes with a bunch of baggage.
It's a bit of a pain in the ass, so I'm moving these fixes here.

]==============================================================]

find_package(DiscordRPC QUIET)
if(DiscordRPC_FOUND)
  return()
endif()

include(FetchContent)

set(BUILD_EXAMPLES OFF CACHE BOOL "Build discord-rpc examples" FORCE)

# Don't run clang-format on downloaded discord-rpc sources.
set(CLANG_FORMAT_CMD OFF)
FetchContent_Declare(discord_rpc
  GIT_REPOSITORY https://github.com/discord/discord-rpc.git
  GIT_TAG 963aa9f3e5ce81a4682c6ca3d136cddda614db33
)

# discord-rpc requires an old cmake
# This is needed for MSYS2 and Homebrew macOS
if(DEFINED CMAKE_POLICY_VERSION_MINIMUM)
  set(nyan_discord_policy_version_minimum_backup "${CMAKE_POLICY_VERSION_MINIMUM}")
  set(nyan_discord_policy_version_minimum_was_defined TRUE)
endif()
if(DEFINED CMAKE_WARN_DEPRECATED)
  set(nyan_discord_warn_deprecated_backup "${CMAKE_WARN_DEPRECATED}")
  set(nyan_discord_warn_deprecated_was_defined TRUE)
endif()
set(CMAKE_POLICY_VERSION_MINIMUM 3.5)
set(CMAKE_WARN_DEPRECATED OFF)
FetchContent_MakeAvailable(discord_rpc)
if(nyan_discord_policy_version_minimum_was_defined)
  set(CMAKE_POLICY_VERSION_MINIMUM "${nyan_discord_policy_version_minimum_backup}")
else()
  unset(CMAKE_POLICY_VERSION_MINIMUM)
endif()
if(nyan_discord_warn_deprecated_was_defined)
  set(CMAKE_WARN_DEPRECATED "${nyan_discord_warn_deprecated_backup}")
else()
  unset(CMAKE_WARN_DEPRECATED)
endif()
unset(nyan_discord_policy_version_minimum_backup)
unset(nyan_discord_policy_version_minimum_was_defined)
unset(nyan_discord_warn_deprecated_backup)
unset(nyan_discord_warn_deprecated_was_defined)
unset(CLANG_FORMAT_CMD)

target_include_directories(discord-rpc INTERFACE
  "${discord_rpc_SOURCE_DIR}/include"
)

# ignore warnings from discord-rpc
# they are loud and noisy
if(MSVC)
  target_compile_options(discord-rpc PRIVATE /w)
else()
  target_compile_options(discord-rpc PRIVATE -w)
endif()

# Fix MSYS2 builds
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 15)
  target_compile_options(discord-rpc PRIVATE -Wno-template-body)
endif()

# Fix macOS Homebrew builds - remove linker warnings
if(APPLE)
  get_target_property(nyan_discord_link_libraries discord-rpc LINK_LIBRARIES)
  get_target_property(nyan_discord_interface_link_libraries discord-rpc INTERFACE_LINK_LIBRARIES)
  string(REPLACE
    "-framework AppKit, -mmacosx-version-min=10.10"
    "-framework AppKit"
    nyan_discord_link_libraries
    "${nyan_discord_link_libraries}"
  )
  string(REPLACE
    "-framework AppKit, -mmacosx-version-min=10.10"
    "-framework AppKit"
    nyan_discord_interface_link_libraries
    "${nyan_discord_interface_link_libraries}"
  )
  set_target_properties(discord-rpc PROPERTIES
    LINK_LIBRARIES "${nyan_discord_link_libraries}"
    INTERFACE_LINK_LIBRARIES "${nyan_discord_interface_link_libraries}"
  )
  unset(nyan_discord_link_libraries)
  unset(nyan_discord_interface_link_libraries)
endif()

add_library(DiscordRPC::discord-rpc ALIAS discord-rpc)
set(DiscordRPC_FOUND TRUE)
