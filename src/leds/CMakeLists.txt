target_sources(firmware PRIVATE color.cpp led_manager.cpp status_leds_handler.cpp)

if(HAS_SIDE_LEDS AND BOARD IN_LIST BUDDY_BOARDS)
  target_sources(firmware PRIVATE side_strip.cpp side_strip_control.cpp)
endif()
