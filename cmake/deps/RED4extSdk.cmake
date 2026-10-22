option(RED4EXT_INSTALL "" OFF)
option(RED4EXT_USE_PCH "" ON)

FetchContent_Declare(
  RED4ext.SDK
  GIT_REPOSITORY  https://github.com/maximegmd/RED4ext.SDK.git
  GIT_TAG         fd1867297719e26c6fbd54734c7ce82448d32c6f # wopss/RED4ext.SDK#227 (game 2.5), switch back once merged
)
FetchContent_MakeAvailable(RED4ext.SDK)

set_target_properties(
  RED4ext.SDK
    PROPERTIES
      FOLDER "Dependencies"
)

mark_as_advanced(
  RED4EXT_BUILD_EXAMPLES
  RED4EXT_HEADER_ONLY
  RED4EXT_INSTALL
  RED4EXT_USE_PCH
)
