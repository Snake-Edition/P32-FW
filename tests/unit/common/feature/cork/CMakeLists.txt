add_compile_definitions(UNITTEST)
add_executable(
  cork_gcode ${CMAKE_CURRENT_SOURCE_DIR}/cork.cpp
             ${CMAKE_SOURCE_DIR}/src/common/feature/cork/tracker.cpp
  )
target_include_directories(
  cork_gcode PUBLIC . ${CMAKE_SOURCE_DIR}/src/common ${CMAKE_SOURCE_DIR}/src/lang
  )

add_catch_test(cork_gcode)
