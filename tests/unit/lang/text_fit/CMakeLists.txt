#
# TEXT FIT TESTS
#

# Extract texts from error yaml
file(GLOB ERROR_TEXT_EXTRACTOR ${PROJECT_ROOT_DIR}/utils/extract_error_texts.py)

add_custom_command(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/errors.hpp
  DEPENDS ${ERROR_TEXT_EXTRACTOR}
          ${PROJECT_ROOT_DIR}/lib/Prusa-Error-Codes/yaml/buddy-error-codes.yaml
          ${PROJECT_ROOT_DIR}/lib/Prusa-Error-Codes/yaml/mmu-error-codes.yaml
  COMMAND
    ${Python3_EXECUTABLE} ${ERROR_TEXT_EXTRACTOR}
    ${PROJECT_ROOT_DIR}/lib/Prusa-Error-Codes/yaml/buddy-error-codes.yaml
    ${PROJECT_ROOT_DIR}/lib/Prusa-Error-Codes/yaml/mmu-error-codes.yaml
  COMMENT "Extracting error texts for unit testing"
  )

add_custom_target(error_texts_extraction DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/errors.hpp)

# Generate hash tables
file(GLOB_RECURSE POFILES ${CMAKE_SOURCE_DIR}/src/lang/po/*/*.po)
add_custom_command(
  OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/hash_table_buckets.ipp"
         "${CMAKE_CURRENT_BINARY_DIR}/hash_table_buckets_count.ipp"
  COMMAND "${Python3_EXECUTABLE}" "${PROJECT_ROOT_DIR}/utils/translations_and_fonts/lang.py"
          "generate-hash-tables" "${CMAKE_SOURCE_DIR}/src/lang/po" "${CMAKE_CURRENT_BINARY_DIR}"
  DEPENDS "${PROJECT_ROOT_DIR}/utils/translations_and_fonts/lang.py" ${POFILES}
  COMMENT "Generating C++ files out of .po files."
  VERBATIM
  )

add_custom_target(
  translation_hashtables DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/hash_table_buckets.ipp"
  )

# Mini display text fit test
add_executable(
  text_fit_tests_mini
  ${CMAKE_CURRENT_SOURCE_DIR}/mini_display_tests.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/text_fit_test.cpp
  ${CMAKE_SOURCE_DIR}/src/common/str_utils.cpp
  ${CMAKE_SOURCE_DIR}/src/lang/string_view_utf8.cpp
  ${CMAKE_SOURCE_DIR}/src/gui/fonts.cpp
  ${CMAKE_SOURCE_DIR}/src/guiapi/src/Rect16.cpp
  ${CMAKE_SOURCE_DIR}/src/lang/translator.cpp
  ${CMAKE_SOURCE_DIR}/src/lang/translation_provider_CPUFLASH.cpp
  )

target_include_directories(
  text_fit_tests_mini
  PRIVATE .
          ${CMAKE_SOURCE_DIR}/src
          ${CMAKE_SOURCE_DIR}/src/common
          ${CMAKE_SOURCE_DIR}/src/common/utils
          ${CMAKE_SOURCE_DIR}/src/lang
          ${CMAKE_BINARY_DIR}/src/lang
          ${CMAKE_SOURCE_DIR}/src/gui
          ${CMAKE_SOURCE_DIR}/src/gui/dialogs
          ${CMAKE_SOURCE_DIR}/src/guiapi/include
          ${PROJECT_ROOT_DIR}/tests/unit/lang/translator
          # Have to be before ${CMAKE_BINARY_DIR}/include otherwise translations options will be not
          # overwritten
          ${CMAKE_SOURCE_DIR}/include
          ${CMAKE_BINARY_DIR}/include
          ${CMAKE_CURRENT_SOURCE_DIR}
          ${CMAKE_CURRENT_BINARY_DIR}
  )

target_compile_definitions(text_fit_tests_mini PUBLIC BOARD=1)

add_dependencies(text_fit_tests_mini translation_hashtables)
add_dependencies(text_fit_tests_mini error_texts_extraction)

target_link_libraries(text_fit_tests_mini SG14)

add_catch_test(text_fit_tests_mini)

# Large display text fit test
add_executable(
  text_fit_tests_large
  ${CMAKE_CURRENT_SOURCE_DIR}/large_display_tests.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/text_fit_test.cpp
  ${CMAKE_SOURCE_DIR}/src/common/str_utils.cpp
  ${CMAKE_SOURCE_DIR}/src/lang/string_view_utf8.cpp
  ${CMAKE_SOURCE_DIR}/src/gui/fonts.cpp
  ${CMAKE_SOURCE_DIR}/src/guiapi/src/Rect16.cpp
  ${CMAKE_SOURCE_DIR}/src/lang/translator.cpp
  ${CMAKE_SOURCE_DIR}/src/lang/translation_provider_CPUFLASH.cpp
  )

target_include_directories(
  text_fit_tests_large
  PRIVATE .
          ${CMAKE_SOURCE_DIR}/src
          ${CMAKE_SOURCE_DIR}/src/common
          ${CMAKE_SOURCE_DIR}/src/common/utils
          ${CMAKE_SOURCE_DIR}/src/lang
          ${CMAKE_BINARY_DIR}/src/lang
          ${CMAKE_SOURCE_DIR}/src/gui
          ${CMAKE_SOURCE_DIR}/src/gui/dialogs
          ${CMAKE_SOURCE_DIR}/src/guiapi/include
          ${CMAKE_SOURCE_DIR}/include
          ${PROJECT_ROOT_DIR}/tests/unit/lang/translator
          # Have to be before ${CMAKE_BINARY_DIR}/include otherwise translations options will be not
          # overwritten
          ${CMAKE_BINARY_DIR}/include
          ${CMAKE_CURRENT_SOURCE_DIR}
          ${CMAKE_CURRENT_BINARY_DIR}
  )

target_compile_definitions(text_fit_tests_large PUBLIC BOARD=2)

add_dependencies(text_fit_tests_large translation_hashtables)
add_dependencies(text_fit_tests_large error_texts_extraction)

target_link_libraries(text_fit_tests_large SG14)

add_catch_test(text_fit_tests_large)
