FetchContent_Declare(
  redscript
  GIT_REPOSITORY  https://github.com/jac3km4/redscript.git
  GIT_TAG         v1.0.0-preview.22
  GIT_SHALLOW     ON
)
FetchContent_MakeAvailable(redscript)

add_library(redscript INTERFACE)

target_include_directories(
  redscript
    INTERFACE
      "${redscript_SOURCE_DIR}/crates/scc/capi/include"
)
