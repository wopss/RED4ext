option(WIL_BUILD_PACKAGING "" OFF)
option(WIL_BUILD_TESTS "" OFF)

FetchContent_Declare(
  WIL
  GIT_REPOSITORY  https://github.com/microsoft/wil.git
  GIT_TAG         3ecba603d9789435b0661be7b7154d45c195ef98
  EXCLUDE_FROM_ALL # The project does not have any INSTALL options.
)
FetchContent_MakeAvailable(WIL)

mark_as_advanced(
  FAST_BUILD
  WIL_BUILD_PACKAGING
  WIL_BUILD_TESTS
  WIL_ENABLE_ASAN
  WIL_ENABLE_UBSAN
)
