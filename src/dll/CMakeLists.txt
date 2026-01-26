add_library(RED4ext.Dll SHARED)

set_target_properties(RED4ext.Dll PROPERTIES OUTPUT_NAME RED4ext)

configure_file(Version.hpp.in "${CMAKE_CURRENT_SOURCE_DIR}/Version.hpp" @ONLY)

file(GLOB_RECURSE HEADER_FILES *.hpp)
file(GLOB_RECURSE SOURCE_FILES *.cpp)
file(GLOB_RECURSE RC_FILES *.rc)

source_group(_CMake REGULAR_EXPRESSION cmake_pch.*)
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${HEADER_FILES} ${SOURCE_FILES} ${RC_FILES})

target_include_directories(RED4ext.Dll PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_sources(RED4ext.Dll PRIVATE ${HEADER_FILES} ${SOURCE_FILES} ${RC_FILES})

target_precompile_headers(RED4ext.Dll PUBLIC stdafx.hpp)
target_link_libraries(RED4ext.Dll
  PUBLIC
    ntdll
    version

    Detours
    fmt
    RED4ext::SDK
    simdjson::simdjson
    spdlog
    toml11
    tsl::ordered_map
    WIL
    redscript
)
