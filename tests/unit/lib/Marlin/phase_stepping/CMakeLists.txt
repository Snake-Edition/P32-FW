add_compile_definitions(UNITTEST)
add_executable(
  Marlin_phase_stepping_tests
  ${CMAKE_CURRENT_SOURCE_DIR}/float_vs_fixed.cpp ${CMAKE_CURRENT_SOURCE_DIR}/lut_old.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/rounding.cpp
  ${CMAKE_SOURCE_DIR}/lib/Marlin/Marlin/src/feature/phase_stepping/lut.cpp
  )
target_include_directories(
  Marlin_phase_stepping_tests
  PUBLIC . ${CMAKE_SOURCE_DIR}/include
         ${CMAKE_SOURCE_DIR}/lib/Marlin/Marlin/src/feature/phase_stepping
  )
add_catch_test(Marlin_phase_stepping_tests)
