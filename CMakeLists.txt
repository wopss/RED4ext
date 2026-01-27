cmake_minimum_required(VERSION 3.26)

# ------------------------------------------------------------------------------
# Project variables
# ------------------------------------------------------------------------------
set(RED4EXT_CMAKE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
set(RED4EXT_CMAKE_DEPS_DIR "${RED4EXT_CMAKE_DIR}/deps")
set(RED4EXT_CMAKE_TEMPLATES_DIR "${RED4EXT_CMAKE_DIR}/templates")

# ------------------------------------------------------------------------------
# Modules
# ------------------------------------------------------------------------------
include("${RED4EXT_CMAKE_DIR}/ConfigureDefaultOutputDirectories.cmake")
include("${RED4EXT_CMAKE_DIR}/ConfigureResourceRc.cmake")
include("${RED4EXT_CMAKE_DIR}/ConfigureVersionFile.cmake")
include("${RED4EXT_CMAKE_DIR}/ConfigureVersionFromGit.cmake")
include("${RED4EXT_CMAKE_DIR}/TargetAppendOutputDirectory.cmake")

# ------------------------------------------------------------------------------
# Versioning
# ------------------------------------------------------------------------------
red4ext_configure_version_from_git()

# ------------------------------------------------------------------------------
# Project definition
# ------------------------------------------------------------------------------
project(
  RED4ext
  VERSION "${RED4EXT_GIT_VERSION_MAJOR}.${RED4EXT_GIT_VERSION_MINOR}.${RED4EXT_GIT_VERSION_PATCH}"
  LANGUAGES CXX
)

# ------------------------------------------------------------------------------
# General configuration
# ------------------------------------------------------------------------------
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELEASE ON)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_MINSIZEREL ON)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELWITHDEBINFO ON)
set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "ProgramDatabase")

red4ext_configure_default_output_directories()

# ------------------------------------------------------------------------------
# Options
# ------------------------------------------------------------------------------
option(RED4EXT_GIT_METADATA "Append Git metadata to 'RED4EXT_GIT_VERSION' CMake variable." ON)

option(RED4EXT_EXTRA_WARNINGS "Enable extra warnings." ON)
option(RED4EXT_TREAT_WARNINGS_AS_ERRORS "Treat compiler warnings as errors." OFF)

# ------------------------------------------------------------------------------
# Toolchain settings for all targets
# ------------------------------------------------------------------------------
add_compile_definitions(
  UNICODE
  _UNICODE
  WIN32_LEAN_AND_MEAN

  # Support Windows 7 and above.
  WINVER=0x0601
  _WIN32_WINNT=0x0601
)

add_link_options(
  /DEBUG:FULL

  $<$<CONFIG:Release,MinSizeRel,RelWithDebInfo>:/INCREMENTAL:NO>
)

# ------------------------------------------------------------------------------
# Dependencies
# ------------------------------------------------------------------------------
include(FetchContent)

include("${RED4EXT_CMAKE_DEPS_DIR}/Detours.cmake")
include("${RED4EXT_CMAKE_DEPS_DIR}/Fmt.cmake")
include("${RED4EXT_CMAKE_DEPS_DIR}/OrderedMap.cmake")
include("${RED4EXT_CMAKE_DEPS_DIR}/RED4extSdk.cmake")
include("${RED4EXT_CMAKE_DEPS_DIR}/Redscript.cmake")
include("${RED4EXT_CMAKE_DEPS_DIR}/Simdjson.cmake")
include("${RED4EXT_CMAKE_DEPS_DIR}/Spdlog.cmake")
include("${RED4EXT_CMAKE_DEPS_DIR}/Toml11.cmake")
include("${RED4EXT_CMAKE_DEPS_DIR}/Wil.cmake")

# ------------------------------------------------------------------------------
# Toolchain settings for this project only
# ------------------------------------------------------------------------------
add_compile_options(
  /GS
  /sdl

  /permissive-
  /Zc:__cplusplus
  /Zc:__STDC__
  /Zc:enumTypes
  /Zc:inline
  /Zc:preprocessor
  /Zc:templateScope

  $<$<CONFIG:Release,MinSizeRel,RelWithDebInfo>:/Gy>
  $<$<CONFIG:Release,MinSizeRel,RelWithDebInfo>:/Oi>

  # Disable some warnings that are not relevant to us.
  /external:I${FETCHCONTENT_BASE_DIR}
  /external:W0
)

if(RED4EXT_EXTRA_WARNINGS)
  add_compile_options(/W4)
endif()

if(RED4EXT_TREAT_WARNINGS_AS_ERRORS)
  add_compile_options(/WX)
endif()

# ------------------------------------------------------------------------------
# Main projects
# ------------------------------------------------------------------------------
add_subdirectory(src)

# ------------------------------------------------------------------------------
# Install
# ------------------------------------------------------------------------------
if(NOT CMAKE_SKIP_INSTALL_RULES)
  set(RED4EXT_INSTALL_X64_DIR bin/x64)
  set(RED4EXT_INSTALL_BIN_DIR red4ext)

  install(
    TARGETS RED4ext.Loader
    RUNTIME DESTINATION "${RED4EXT_INSTALL_X64_DIR}"
  )

  install(
    FILES $<TARGET_PDB_FILE:RED4ext.Loader>
    DESTINATION "${RED4EXT_INSTALL_X64_DIR}"
  )

  install(
    TARGETS RED4ext.Dll
    RUNTIME DESTINATION "${RED4EXT_INSTALL_BIN_DIR}"
  )

  install(
    FILES $<TARGET_PDB_FILE:RED4ext.Dll>
    DESTINATION "${RED4EXT_INSTALL_BIN_DIR}"
  )

  install(
    FILES "${PROJECT_SOURCE_DIR}/LICENSE.md"
    DESTINATION "${RED4EXT_INSTALL_BIN_DIR}"
    RENAME LICENSE.txt
  )

  install(
    FILES "${PROJECT_SOURCE_DIR}/THIRD_PARTY_LICENSES.md"
    DESTINATION "${RED4EXT_INSTALL_BIN_DIR}"
    RENAME THIRD_PARTY_LICENSES.txt
  )
endif()
