add_library(RED4ext.Playground SHARED)

red4ext_target_append_output_directory(
  RED4ext.Playground
  plugins
)

red4ext_configure_resource_rc(
  RED4ext.Playground
  "A playground plugin for ${PROJECT_NAME}."
)

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
  RED4ext.Playground
    PRIVATE
      "${CMAKE_CURRENT_SOURCE_DIR}"
)

target_precompile_headers(
  RED4ext.Playground
    PRIVATE
      stdafx.hpp
)

target_sources(
  RED4ext.Playground
    PRIVATE
      ${RED4EXT_FILES}
      ${RED4EXT_RC_FILE}
)

target_link_libraries(
  RED4ext.Playground
    PRIVATE
      fmt::fmt
      RED4ext::SDK
)
