add_subdirectory(filament)
add_subdirectory(toolhead)

target_sources(firmware PRIVATE screen_factory_reset.cpp screen_m600.cpp screen_preheat.cpp)

if(HAS_CHAMBER_FILTRATION_API)
  target_sources(firmware PRIVATE screen_chamber_filtration.cpp)
endif()

if(HAS_CANCEL_OBJECT)
  target_sources(firmware PRIVATE screen_cancel_objects.cpp)
endif()

if(HAS_MMU2)
  target_sources(firmware PRIVATE screen_hw_mmu.cpp)
endif()
