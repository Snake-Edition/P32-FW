add_subdirectory(factory_reset)

if(HAS_BELT_TUNING)
  add_subdirectory(belt_tuning)
endif()

if(HAS_XBUDDY_EXTENSION)
  add_subdirectory(xbuddy_extension)
endif()

if(HAS_CHAMBER_API)
  add_subdirectory(chamber)
endif()

if(HAS_CHAMBER_FILTRATION_API)
  add_subdirectory(chamber_filtration)
endif()

if(HAS_EMERGENCY_STOP)
  add_subdirectory(emergency_stop)
endif()

if(HAS_CEILING_CLEARANCE)
  add_subdirectory(ceiling_clearance)
endif()

add_subdirectory(print_status_message)

add_subdirectory(ramming)
