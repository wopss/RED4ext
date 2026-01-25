add_library(RED4ext.Loader SHARED)

set_target_properties(
  RED4ext.Loader
    PROPERTIES
      OUTPUT_NAME winmm
)

red4ext_configure_resource_rc(
  RED4ext.Loader
  "The loader for ${PROJECT_NAME}."
)

file(
  GLOB_RECURSE
    RED4EXT_FILES
      *.hpp
      *.cpp
      *.def

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
)

target_include_directories(
  RED4ext.Loader
    PRIVATE
      "${CMAKE_CURRENT_SOURCE_DIR}"
)

target_precompile_headers(
  RED4ext.Loader
    PRIVATE
      stdafx.hpp
)

target_sources(
  RED4ext.Loader
    PRIVATE
      ${RED4EXT_FILES}
      ${RED4EXT_RC_FILE}
)

target_link_libraries(
  RED4ext.Loader
    PRIVATE
      fmt::fmt
      WIL::WIL
)
