function(red4ext_configure_version_file TARGET)
  set(OUTPUT_FILE "${CMAKE_CURRENT_BINARY_DIR}/Version.cpp")

  configure_file(
    "${RED4EXT_CMAKE_TEMPLATES_DIR}/Version.cpp.in"
    "${OUTPUT_FILE}"
    @ONLY
  )

  set(RED4EXT_VERSION_FILE "${OUTPUT_FILE}" PARENT_SCOPE)
endfunction()
