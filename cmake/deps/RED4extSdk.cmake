option(RED4EXT_INSTALL "" OFF)
option(RED4EXT_USE_PCH "" ON)

FetchContent_Declare(
  RED4ext.SDK
  GIT_REPOSITORY  https://github.com/wopss/RED4ext.SDK.git
  GIT_TAG         0f5eba42a4cf3a1161ad3de0b4ed6f1901ec169e
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
