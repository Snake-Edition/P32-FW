target_sources(firmware PRIVATE print_status_message_guard.cpp print_status_message_mgr.cpp)

if(HAS_GUI)
  target_sources(firmware PRIVATE print_status_message_formatter_buddy.cpp)
endif()
