option(WIL_BUILD_PACKAGING "" OFF)
option(WIL_BUILD_TESTS "" OFF)

FetchContent_Declare(
  WIL
  GIT_REPOSITORY  https://github.com/microsoft/wil.git
  GIT_TAG         ff206266eddc3d1f788e11db472d9f28d799e08f
  EXCLUDE_FROM_ALL # The project does not have any INSTALL options.
)
FetchContent_MakeAvailable(WIL)

mark_as_advanced(
  FAST_BUILD
  WIL_BUILD_PACKAGING
  WIL_BUILD_TESTS
)
