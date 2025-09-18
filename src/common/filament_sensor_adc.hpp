/**
 * @file filament_sensor_adc.hpp
 * @author Radek Vana
 * @brief basic api of analog filament sensor
 * @date 2019-12-16
 */

#pragma once

#include "filament_sensor.hpp"
#include <utils/timing/rate_limiter.hpp>

struct metric_s;

class FSensorADC final : public IFSensor {

protected:
    int32_t fs_ref_ins_value { 0 }; ///< value of filament insert in extruder
    int32_t fs_ref_nins_value { 0 }; ///< value of filament not inserted in extruder

    std::atomic<int32_t> fs_filtered_value; ///< current filtered value set from interrupt
    static_assert(std::atomic<decltype(fs_filtered_value)::value_type>::is_always_lock_free, "Lock free type must be used from ISR.");

    /**
     * @brief Get filtered sensor-specific value.
     * @return filtered ADC value
     */
    virtual int32_t GetFilteredValue() const override { return fs_filtered_value.load(); };

    const uint8_t tool_index;
    const bool is_side;

    CalibrateRequest req_calibrate { CalibrateRequest::NoCalibration };
    bool flg_invalid_calib { false };

    virtual void cycle() override;

    /**
     * @brief Calibrate reference value with filament NOT inserted
     */
    void CalibrateNotInserted(int32_t filtered_value);

    /**
     * @brief Calibrate reference value with filament inserted
     */
    void CalibrateInserted(int32_t filtered_value);

    virtual void record_state() override;

public:
    FSensorADC(uint8_t tool_index, bool is_side_sensor);

    /**
     * @brief calibrate filament sensor and store it to eeprom
     * thread safe, only sets flag --> !!! is not done instantly !!!
     */
    virtual void SetCalibrateRequest(CalibrateRequest) override;
    virtual bool IsCalibrationFinished() const override;
    virtual void SetInvalidateCalibrationFlag() override;

    void load_settings();

    void set_filtered_value_from_IRQ(int32_t filtered_value);

    void invalidate_calibration();

private:
    // Limit metrics recording for each tool
    RateLimiter<uint32_t> limit_record;

    uint32_t value_span = 0;

    static constexpr float fs_selftest_span_multipler { 1.2 };
};
