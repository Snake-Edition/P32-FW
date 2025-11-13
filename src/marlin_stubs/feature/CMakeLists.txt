if(HAS_CHAMBER_API)
  add_subdirectory(chamber)
endif()

if(HAS_CHAMBER_VENTS)
  add_subdirectory(automatic_chamber_vents)
endif()
