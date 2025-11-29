#pragma once

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

extern uint32_t heap_total_size();
extern uint32_t heap_bytes_remaining();

uint32_t mem_is_heap_allocated(const void *ptr);

// Malloc, but returns null on failure, not a redscreen.
void *malloc_fallible(size_t size);

void setup_isr_stack_overflow_trap();
void check_isr_stack_overflow();

#ifdef __cplusplus
}
#endif //__cplusplus
