option(RED4EXT_INSTALL "" OFF)
option(RED4EXT_USE_PCH "" ON)

FetchContent_Declare(
  RED4ext.SDK
  GIT_REPOSITORY  https://github.com/wopss/RED4ext.SDK.git
  GIT_TAG         cfb8421cdcf31470c6f3c1705631ea75d8c1aa7f
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
