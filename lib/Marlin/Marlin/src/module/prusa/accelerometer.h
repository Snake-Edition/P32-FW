/**
 * @file
 */
#pragma once

#include "../../inc/MarlinConfigPre.h"
#include <option/has_local_accelerometer.h>
#include <option/has_remote_accelerometer.h>

static_assert(HAS_LOCAL_ACCELEROMETER() || HAS_REMOTE_ACCELEROMETER());

#if HAS_LOCAL_ACCELEROMETER()
    #include <hwio_pindef.h>
#elif HAS_REMOTE_ACCELEROMETER()
    #include <freertos/mutex.hpp>
    #include <common/circular_buffer.hpp>
    #include <puppies/fifo_coder.hpp>
#else
    #error "Why do you #include me?"
#endif

/**
 * This class must not be instantiated globally, because (for MK3.5) it temporarily takes
 * ownership of the tachometer pin and turns it into accelerometer chip select pin.
 */
class PrusaAccelerometer {
public:
    struct Acceleration {
        float val[3];
    };

    enum class Error {
        none,
        communication,
#if HAS_REMOTE_ACCELEROMETER()
        no_active_tool,
        busy,
#endif
        overflow_sensor, ///< Data not consistent, sample overrun on accelerometer sensor
#if HAS_REMOTE_ACCELEROMETER()
        overflow_buddy, ///< Data not consistent, sample missed on buddy
        overflow_dwarf, ///< Data not consistent, sample missed on dwarf
        overflow_possible, ///< Data not consistent, sample possibly lost in transfer
#endif

        _cnt
    };

    PrusaAccelerometer();
    ~PrusaAccelerometer();

    /**
     * @brief Clear buffers and Overflow
     */
    void clear();

    enum class GetSampleResult {
        ok,
        buffer_empty,
        error,
    };

    /// Obtains one sample from the buffer and puts it to \param acceleration (if the results is ok).
    GetSampleResult get_sample(Acceleration &acceleration);

    float get_sampling_rate() const;
    /**
     * @brief Get error
     *
     * Check after PrusaAccelerometer construction.
     * Check after measurement to see if it was valid.
     */
    Error get_error() const;

    /// \returns string describing the error or \p nullptr
    const char *error_str() const;

    /// If \p get_error() is not \p None, calls \p report_func(error_str)
    /// \returns \p true if there was an error and \p report_func was called
    template <typename F>
    inline bool report_error(const F &report_func) const {
        if (const auto str = error_str()) {
            report_func(str);
            return true;
        } else {
            return false;
        }
    }

#if HAS_REMOTE_ACCELEROMETER()
    static void put_sample(common::puppies::fifo::AccelerometerXyzSample sample);
    static void set_rate(float rate);
    static void set_possible_overflow();
#endif

private:
    class ErrorImpl {
    public:
        ErrorImpl()
            : m_error(Error::none) {}
        void set(Error error) {
            if (Error::none == m_error) {
                m_error = error;
            }
        }
        Error get() const {
            return m_error;
        }
        void clear_overflow() {
            switch (m_error) {
            case Error::none:
            case Error::communication:
            case Error::_cnt:
#if HAS_REMOTE_ACCELEROMETER()
            case Error::no_active_tool:
            case Error::busy:
#endif
                break;

            case Error::overflow_sensor:
#if HAS_REMOTE_ACCELEROMETER()
            case Error::overflow_buddy:
            case Error::overflow_dwarf:
            case Error::overflow_possible:
#endif
                m_error = Error::none;
                break;
            }
        }

    private:
        Error m_error;
    };

    void set_enabled(bool enable);
#if HAS_LOCAL_ACCELEROMETER() && PRINTER_IS_PRUSA_MK3_5()
    buddy::hw::OutputEnabler output_enabler;
    buddy::hw::OutputPin output_pin;
#elif HAS_REMOTE_ACCELEROMETER()
    // Mutex is very RAM (80B) consuming for this fast operation, consider switching to critical section
    static freertos::Mutex s_buffer_mutex;
    struct SampleBuffer {
        CircularBuffer<common::puppies::fifo::AccelerometerXyzSample, 128> buffer;
        ErrorImpl error;
    };
    static SampleBuffer *s_sample_buffer;
    SampleBuffer m_sample_buffer;
    static float m_sampling_rate;
#endif
};
