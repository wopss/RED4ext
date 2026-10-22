option(RED4EXT_INSTALL "" OFF)
option(RED4EXT_USE_PCH "" ON)

FetchContent_Declare(
  RED4ext.SDK
  GIT_REPOSITORY  https://github.com/alphanin9/RED4ext.SDK.git
  GIT_TAG         849447311af42fbf7f14e9d428b0abd7142bebbd # patch-2.5-compatibility-sdk1.0.0
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
