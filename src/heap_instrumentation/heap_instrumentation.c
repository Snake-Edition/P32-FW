/// @file
///
/// To instrument the heap, we wrap malloc(), free(), calloc() and realloc()
/// and output their parameters. We use separate RTT buffer to get this data
/// from the printer. Data are printed in format suitable for heaplog_viewer
///
/// https://github.com/cesanta/mongoose-os/tree/master/tools/heaplog_viewer
///
/// Visual Studio Code is already setup to read the RTT buffer and dump it to
/// heap_instrumentation.txt file in the root directory.
///
/// You can also use openocd to read the buffer:
///
///     # Find RTT control structure in MCU memory
///     OPENOCD_PORT=...
///     echo 'rtt setup 0x20000000 114687 "SEGGER RTT" | telnet localhost ${OPENOCD_PORT}
///
///     # Start transfer
///     echo 'rtt start' | telnet localhost ${OPENOCD_PORT}
///
///     # Start a TCP server streaming data from buffer 1
///     RTT_PORT=...
///     echo "rtt server start ${RTT_PORT} 1" | telnet localhost ${OPENOCD_PORT}
///
///     # Access the data
///     telnet localhost ${RTT_PORT}

#include "SEGGER_RTT.h"
#include <device/board.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

/// Different boards have different MCUs and those have different RAM size
#define RAM_START "0x20000000"
#if BOARD_IS_BUDDY()
    #define RAM_END "0x20020000"
#elif BOARD_IS_XBUDDY() || BOARD_IS_XLBUDDY()
    #define RAM_END "0x20030000"
#else
    #error "unknown board"
#endif

#define HEADER "hlog_param:{\"heap_start\":\"" RAM_START "\",\"heap_end\":\"" RAM_END "\"}\n"

static unsigned rtt_buffer_index = 0;
static char rtt_buffer_data[256];

__attribute__((format(printf, 1, 2))) static void rtt_buffer_printf(const char *fmt, ...) {
    if (!rtt_buffer_index) {
        rtt_buffer_index = 1;
        SEGGER_RTT_ConfigUpBuffer(rtt_buffer_index, "heap", &rtt_buffer_data[0], sizeof(rtt_buffer_data), SEGGER_RTT_MODE_BLOCK_IF_FIFO_FULL);
        SEGGER_RTT_Write(rtt_buffer_index, HEADER, strlen(HEADER));
    }

    va_list args;
    va_start(args, fmt);
    char buffer[32];
    int n = vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);
    SEGGER_RTT_Write(rtt_buffer_index, buffer, n);
}

void *__wrap_malloc(size_t size) {
    void *__real_malloc(size_t size);
    void *result = __real_malloc(size);
    rtt_buffer_printf("hl{m,%d,0,0x%p}\n", size, result);
    return result;
}

void __wrap_free(void *ptr) {
    void __real_free(void *ptr);
    __real_free(ptr);
    if (ptr) {
        rtt_buffer_printf("hl{f,0x%p}\n", ptr);
    }
}

void *__wrap_calloc(size_t nmemb, size_t size) {
    void *__real_calloc(size_t nmemb, size_t size);
    void *result = __real_calloc(nmemb, size);
    rtt_buffer_printf("hl{c,%d,0,0x%p}\n", nmemb * size, result);
    return result;
}

void *__wrap_realloc(void *ptr, size_t size) {
    void *__real_realloc(void *ptr, size_t size);
    void *result = __real_realloc(ptr, size);
    rtt_buffer_printf("hl{r,%d,0,0x%p}\n", size, result);
    return result;
}
