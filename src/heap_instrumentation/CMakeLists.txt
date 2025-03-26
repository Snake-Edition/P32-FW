target_link_options(
  firmware PRIVATE -Wl,--wrap=malloc -Wl,--wrap=free -Wl,--wrap=calloc -Wl,--wrap=realloc
  )
target_sources(firmware PRIVATE heap_instrumentation.c)
