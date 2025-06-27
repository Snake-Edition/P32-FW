# timer_event_period_tracker_tests
add_executable(
  timer_event_period_tracker_tests
  ${CMAKE_CURRENT_SOURCE_DIR}/timer_event_period_tracker_tests.cpp
  ${CMAKE_SOURCE_DIR}/src/module/utils/utils/timing/timer_event_period_tracker.cpp
  )

target_include_directories(
  timer_event_period_tracker_tests PUBLIC ${CMAKE_SOURCE_DIR}/src/module/utils
  )

add_catch_test(timer_event_period_tracker_tests)

# rate_limiter_tests
add_executable(rate_limiter_tests ${CMAKE_CURRENT_SOURCE_DIR}/rate_limiter_tests.cpp)

target_include_directories(rate_limiter_tests PUBLIC ${CMAKE_SOURCE_DIR}/src/module/utils)

add_catch_test(rate_limiter_tests)

# combo_counter_tests
add_executable(combo_counter_tests ${CMAKE_CURRENT_SOURCE_DIR}/combo_counter_tests.cpp)

target_include_directories(combo_counter_tests PUBLIC ${CMAKE_SOURCE_DIR}/src/module/utils)

add_catch_test(combo_counter_tests)
