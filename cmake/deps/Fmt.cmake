FetchContent_Declare(
  fmt
  GIT_REPOSITORY  https://github.com/fmtlib/fmt.git
  GIT_TAG         12.1.0
  GIT_SHALLOW     ON
)
FetchContent_MakeAvailable(fmt)

set_target_properties(
  fmt
    PROPERTIES
      FOLDER "Dependencies"
)

mark_as_advanced(
  FMT_CUDA_TEST
  FMT_DEBUG_POSTFIX
  FMT_DOC
  FMT_FUZZ
  FMT_INC_DIR
  FMT_INSTALL
  FMT_MODULE
  FMT_OS
  FMT_PEDANTIC
  FMT_SYSTEM_HEADERS
  FMT_TEST
  FMT_UNICODE
  FMT_WERROR
)
