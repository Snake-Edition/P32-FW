# Common utilities directory, with all the nice things that one might want to use elsewhere

add_library(buddy_utils OBJECT utils/enum_array.cpp utils/timing/timer_event_period_tracker.cpp)

target_include_directories(buddy_utils PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
