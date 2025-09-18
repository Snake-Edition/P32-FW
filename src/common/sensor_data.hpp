#pragma once

#include <buddy/door_sensor.hpp>
#include <device/board.h>
#include <option/has_door_sensor.h>
#include <option/has_loadcell.h>
#include <option/has_remote_bed.h>
#include <utils/atomic.hpp>

// this struct collects data from metrics and gives access to the
// sensor info screen
struct SensorData {
private:
    friend SensorData &sensor_data();

    SensorData() = default;
    SensorData(SensorData &other) = delete;
    SensorData(SensorData &&other) = delete;

public:
    RelaxedAtomic<float> MCUTemp;
    RelaxedAtomic<float> boardTemp;
    RelaxedAtomic<float> hbrFan;
    RelaxedAtomic<float> inputVoltage;
#if BOARD_IS_XLBUDDY()
    RelaxedAtomic<float> sandwich5VVoltage;
    RelaxedAtomic<float> sandwich5VCurrent;
    RelaxedAtomic<float> buddy5VCurrent;
    RelaxedAtomic<float> dwarfBoardTemperature;
    RelaxedAtomic<float> dwarfMCUTemperature;
#elif BOARD_IS_XBUDDY()
    RelaxedAtomic<float> heaterVoltage;
    RelaxedAtomic<float> heaterCurrent;
    RelaxedAtomic<float> inputCurrent;
    RelaxedAtomic<float> mmuCurrent;
#else
#endif
#if HAS_DOOR_SENSOR()
    RelaxedAtomic<buddy::DoorSensor::DetailedState> door_sensor_detailed_state;
#endif
#if HAS_LOADCELL()
    RelaxedAtomic<float> loadCell;
#endif
#if HAS_REMOTE_BED()
    RelaxedAtomic<float> bedMCUTemperature;
#endif

    void update();
};

SensorData &sensor_data();
