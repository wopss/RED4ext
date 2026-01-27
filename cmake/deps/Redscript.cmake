FetchContent_Declare(
  redscript
  GIT_REPOSITORY  https://github.com/jac3km4/redscript.git
  GIT_TAG         b413ba6a40098191d18bb24d1a095dd00190322e
)
FetchContent_MakeAvailable(redscript)

add_library(redscript INTERFACE)

target_include_directories(
  redscript
    INTERFACE
      "${redscript_SOURCE_DIR}/crates/scc/capi/include"
)
