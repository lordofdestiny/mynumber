if(NOT EXISTS "${VERIFY_FILE}")
  message(FATAL_ERROR "Expected native artifact does not exist: ${VERIFY_FILE}")
endif()

get_filename_component(VERIFY_EXT "${VERIFY_FILE}" EXT)
if(NOT VERIFY_EXT STREQUAL "${VERIFY_EXT_EXPECTED}")
  message(FATAL_ERROR "Expected ${VERIFY_EXT_EXPECTED} but got ${VERIFY_FILE}")
endif()

message(STATUS "Verified native artifact: ${VERIFY_FILE}")
