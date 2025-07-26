add_library(redscript INTERFACE)

set(REDSCRIPT_SRC_DIR "${PROJECT_SOURCE_DIR}/deps/redscript/crates/scc/capi/include")
set_target_properties(redscript PROPERTIES FOLDER "Dependencies")

target_include_directories(redscript INTERFACE ${REDSCRIPT_SRC_DIR})
