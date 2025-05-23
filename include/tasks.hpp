#pragma once

#include <concepts>
#include <cstdint>
#include <FreeRTOS.h>
#include <event_groups.h>
#include <stdint.h>
#include <option/has_puppies.h>
#include "utility_extensions.hpp"

namespace TaskDeps {

/**
 * Code in this module handles task/component dependencies
 *
 * Provided tools enable definition of dependency masks, waiting for dependencies being satisfied and marking dependencies as provided.
 */

using dependency_t = EventBits_t;

/// Definition of different dependencies
enum class Dependency : size_t {
    puppies_ready,
    resources_ready,
    usb_device_ready,
    default_task_ready,
    esp_flashed,
    networking_ready,
    usb_temp_gui_ready, ///< Check autoprint and powerpanic state
    gui_display_ready, ///< Display is initialized and ready
    gui_ready, ///< GUI is ready, home screen has been opened
    _count
};

// Check dependency mask fits into the dependency_t integer
static_assert(ftrstd::to_underlying(Dependency::_count) <= sizeof(dependency_t) * 8);

// Create dependency mask from the dependencies enum
constexpr dependency_t make(std::same_as<Dependency> auto... dependencies) {
    // Feel free to lift the assert in case some build configuration results in empty list
#if HAS_PUPPIES()
    static_assert(sizeof...(dependencies) > 0, "No dependencies, is this intended?");
#endif
    return ((1 << ftrstd::to_underlying(dependencies)) | ... | 0);
}

/// Definitions of dependencies for different tasks/components
namespace Tasks {
    inline constexpr dependency_t usb_device_start = make(Dependency::usb_device_ready);

    inline constexpr dependency_t default_start = make(
#if HAS_PUPPIES()
        Dependency::puppies_ready
#endif
    );
    inline constexpr dependency_t marlin_client = make(Dependency::default_task_ready);
    inline constexpr dependency_t puppy_run = make(Dependency::default_task_ready);
    inline constexpr dependency_t bootstrap_done = make(
        Dependency::resources_ready,
        Dependency::esp_flashed
#if HAS_PUPPIES()
        ,
        Dependency::puppies_ready
#endif
    );
    inline constexpr dependency_t connect = make(Dependency::networking_ready);
    inline constexpr dependency_t syslog = make(Dependency::networking_ready);
    inline constexpr dependency_t network = make(Dependency::esp_flashed);
    inline constexpr dependency_t bootstrap_start = make(Dependency::gui_display_ready);
    inline constexpr dependency_t puppy_task_start = make(Dependency::esp_flashed);

} // namespace Tasks

// Needed for inline methods being embedded to different compilation modules
extern EventGroupHandle_t components_ready;

/// Initialize component dependency resolution
void components_init();

/**
 * Check whether all dependencies are fulfilled already
 */
inline bool check(dependency_t dependencies) {
    return (xEventGroupGetBits(components_ready) & dependencies) == dependencies;
}

/**
 * Check whether a dependency is fulfilled
 */
inline bool check(Dependency dependency) {
    return check(make(dependency));
}

/**
 * Wait for dependencies of the task/component
 */
inline void wait(dependency_t dependencies) {
    if (dependencies) {
        xEventGroupWaitBits(components_ready, dependencies, pdFALSE, pdTRUE, portMAX_DELAY);
    }
}

/**
 * Mark particular dependency as satisfied
 */
inline void provide(Dependency dependency) {
    xEventGroupSetBits(components_ready, 1 << ftrstd::to_underlying(dependency));
}

} // namespace TaskDeps
