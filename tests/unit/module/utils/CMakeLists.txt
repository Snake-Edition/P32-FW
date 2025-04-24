add_executable(
  timer_event_period_tracker_tests
  ${CMAKE_CURRENT_SOURCE_DIR}/timer_event_period_tracker_tests.cpp
  ${CMAKE_SOURCE_DIR}/src/module/utils/utils/timing/timer_event_period_tracker.cpp
  )

target_include_directories(
  timer_event_period_tracker_tests PUBLIC ${CMAKE_SOURCE_DIR}/src/module/utils
  )

add_catch_test(timer_event_period_tracker_tests)
