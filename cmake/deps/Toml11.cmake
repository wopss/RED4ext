option(BUILD_TESTING "" OFF)

FetchContent_Declare(
  toml11
  GIT_REPOSITORY  https://github.com/ToruNiina/toml11.git
  GIT_TAG         v4.4.0
  GIT_SHALLOW     ON
)
FetchContent_MakeAvailable(toml11)

mark_as_advanced(
  BUILD_TESTING
  TOML11_ENABLE_ACCESS_CHECK
  TOML11_PRECOMPILE
)
