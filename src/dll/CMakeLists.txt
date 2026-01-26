add_library(RED4ext.Dll SHARED)

set_target_properties(
  RED4ext.Dll
    PROPERTIES
      OUTPUT_NAME ${PROJECT_NAME}
)

red4ext_configure_resource_rc(
  RED4ext.Dll
  "A script extender for REDengine 4."
)

red4ext_configure_version_file(RED4ext.Dll)

file(
  GLOB_RECURSE
    RED4EXT_FILES
      *.hpp
      *.cpp
)

source_group(CMake REGULAR_EXPRESSION cmake_pch.*)

source_group(
  TREE
    "${CMAKE_CURRENT_SOURCE_DIR}"
  FILES
    ${RED4EXT_FILES}
)

source_group(
  TREE
    "${CMAKE_CURRENT_BINARY_DIR}"
  FILES
    ${RED4EXT_RC_FILE}
    ${RED4EXT_VERSION_FILE}
)

target_include_directories(
  RED4ext.Dll
    PRIVATE
      "${CMAKE_CURRENT_SOURCE_DIR}"
)

target_precompile_headers(
  RED4ext.Dll
    PRIVATE
      stdafx.hpp
)

target_sources(
  RED4ext.Dll
    PRIVATE
      ${RED4EXT_FILES}
      ${RED4EXT_RC_FILE}
      ${RED4EXT_VERSION_FILE}
)

target_link_libraries(
  RED4ext.Dll
    PRIVATE
      ntdll
      version

      Detours
      fmt::fmt
      RED4ext::SDK
      redscript
      simdjson::simdjson
      spdlog::spdlog
      toml11::toml11
      tsl::ordered_map
      WIL::WIL
)
