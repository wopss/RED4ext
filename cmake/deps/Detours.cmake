FetchContent_Declare(
  Detours
  GIT_REPOSITORY  https://github.com/microsoft/Detours.git
  GIT_TAG         9764cebcb1a75940e68fa83d6730ffaf0f669401
)
FetchContent_MakeAvailable(Detours)

add_library(Detours STATIC)
set_target_properties(
  Detours
    PROPERTIES
      FOLDER "Dependencies"
)

file(GLOB_RECURSE HEADER_FILES "${detours_SOURCE_DIR}/src/*.h")
file(GLOB_RECURSE SOURCE_FILES "${detours_SOURCE_DIR}/src/*.cpp")

# Remove "uimports.cpp" since it throws "detours.h version mismatch" error.
list(REMOVE_ITEM SOURCE_FILES "${detours_SOURCE_DIR}/src/uimports.cpp")

target_include_directories(
  Detours
    PUBLIC
      "${detours_SOURCE_DIR}/src"
)

target_sources(
  Detours
    PRIVATE
      "${HEADER_FILES}"
      "${SOURCE_FILES}"
)
