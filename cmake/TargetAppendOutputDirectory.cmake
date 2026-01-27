function(red4ext_target_append_output_directory TARGET DIRECTORY)
  __red4ext_target_append_directory_if_property_set(
    ${TARGET}
    ARCHIVE_OUTPUT_DIRECTORY
    "${DIRECTORY}"
  )

  __red4ext_target_append_directory_if_property_set(
    ${TARGET}
    LIBRARY_OUTPUT_DIRECTORY
    "${DIRECTORY}"
  )

  __red4ext_target_append_directory_if_property_set(
    ${TARGET}
    RUNTIME_OUTPUT_DIRECTORY
    "${DIRECTORY}"
  )

 foreach(CONFIGURATION ${CMAKE_CONFIGURATION_TYPES})
    string(TOUPPER ${CONFIGURATION} CONFIGURATION_UPPER)

    __red4ext_target_append_directory_if_property_set(
      ${TARGET}
      "ARCHIVE_OUTPUT_DIRECTORY_${CONFIGURATION_UPPER}"
      "${DIRECTORY}"
    )

    __red4ext_target_append_directory_if_property_set(
      ${TARGET}
      "LIBRARY_OUTPUT_DIRECTORY_${CONFIGURATION_UPPER}"
      "${DIRECTORY}"
    )

    __red4ext_target_append_directory_if_property_set(
      ${TARGET}
      "RUNTIME_OUTPUT_DIRECTORY_${CONFIGURATION_UPPER}"
      "${DIRECTORY}"
    )
  endforeach()
endfunction()

function(__red4ext_target_append_directory_if_property_set TARGET PROPERTY DIRECTORY)
  get_target_property(CURRENT_PROPERTY_VALUE ${TARGET} ${PROPERTY})
  if(CURRENT_PROPERTY_VALUE)
    set_target_properties(
      ${TARGET}
      PROPERTIES
        ${PROPERTY} "${CURRENT_PROPERTY_VALUE}/${DIRECTORY}"
    )
  endif()
endfunction()
