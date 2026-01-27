FetchContent_Declare(
  ordered_map
  GIT_REPOSITORY  https://github.com/Tessil/ordered-map.git
  GIT_TAG         5bbd64709218b9f9fc92bf2bbbf7cae6f5e067b6
  EXCLUDE_FROM_ALL # The project does not have any INSTALL options.
)
FetchContent_MakeAvailable(ordered_map)

target_compile_definitions(
  ordered_map
    INTERFACE
      NOMINMAX
)
