#pragma once

#include "lfs.h"

#if defined(__cplusplus)
extern "C" {
#endif // defined(__cplusplus)

/// Initialize the littlefs filesystem
lfs_t *littlefs_internal_init();

#if defined(__cplusplus)
} // extern "C"
#endif // defined(__cplusplus)
