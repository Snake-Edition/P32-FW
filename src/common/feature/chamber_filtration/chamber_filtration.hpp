#pragma once

#include <array>

#include <pwm_utils.hpp>
#include <freertos/mutex.hpp>
#include <general_response.hpp>

#include "chamber_filtration_enums.hpp"

namespace buddy {

/// API for controlling chamber filtration (filtering the fumes out during & after print)
/// The API is thread-safe
class ChamberFiltration {

public:
    static constexpr size_t max_backend_count = 4;
    static constexpr size_t max_post_print_filtration_time_min = 90; // Propagate any change to Connect

    using Backend = ChamberFiltrationBackend;
    using BackendArray = std::array<Backend, max_backend_count>;

    /// \returns translatable name of the provided filtration backend
    static const char *backend_name(Backend backend);

    /// Stores all available backends in an UI-friendly manner into the target memory (including "none")
    /// \returns number of backends
    static size_t get_available_backends(BackendArray &target);

public:
    /// \returns PWM the filtration fan should be driven with
    /// This is the minimum PWM the fan should be running at - if the fan serves some other purpose as well, you can use std::max
    PWM255 output_pwm() const;

    /// \returns the current backend that should be using the filtration API
    ChamberFiltrationBackend backend() const;

    void set_backend(ChamberFiltrationBackend backend);

    bool is_enabled() const {
        return backend() != ChamberFiltrationBackend::none;
    }

    void step();

    /// \brief Enables/disables the filtration for the current print
    /// \param needs_filtration true if the filtration should be forced on (ignore filament needs), false if it should be forced off
    void set_needs_filtration(bool needs_filtration);

public:
    /// \returns rated lifetime of the HEPA filter in seconds. 0 if not unknown/not specified
    uint32_t filter_lifetime_s() const;

    /// Check HEPA filter expiration and possibly show warning
    void check_filter_expiration();

    /// Process filter change, reset timers and such
    void change_filter();

    /// Processes response from WarningType::EnclosureFilterExpiration
    void handle_filter_expiration_warning(Response response);

private:
    void update_needs_filtration();

private:
    mutable freertos::Mutex mutex_;

    PWM255 output_pwm_;
    bool is_printing_prev_ = false;
    std::optional<bool> needs_filtration_;
    uint32_t last_print_s_ = 0;

    /// ticks_s() of the start of filter usage (output_pwm > 0) that has not yet been emitted in the config_store
    uint32_t unaccounted_filter_time_used_start_s_ = 0;
};

ChamberFiltration &chamber_filtration();

} // namespace buddy
