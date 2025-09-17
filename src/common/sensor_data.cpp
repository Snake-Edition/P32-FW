#include <common/sensor_data.hpp>
#include <printers.h>

#include <option/has_door_sensor.h>
#include <option/has_mmu2.h>
#include <option/has_advanced_power.h>
#if HAS_ADVANCED_POWER()
    #include "advanced_power.hpp"
#endif // HAS_ADVANCED_POWER()

#include <adc.hpp>
#include "../Marlin/src/module/temperature.h"

#if BOARD_IS_XLBUDDY()
    #include <Marlin/src/module/prusa/toolchanger.h>
#endif

SensorData &sensor_data() {
    static SensorData instance;
    return instance;
}

void SensorData::update() {
    // Board temperature
    boardTemp = thermalManager.degBoard();

#if HAS_DOOR_SENSOR()
    // Door sensor
    door_sensor_detailed_state = buddy::door_sensor().detailed_state();
#endif

    // Heatbreak fan speed
    hbrFan = thermalManager.fan_speed[1];

#if BOARD_IS_XBUDDY()

    // Input voltage
    inputVoltage = advancedpower.GetBedVoltage();

    // Heater voltage
    heaterVoltage = advancedpower.GetHeaterVoltage();

    // Heater current
    heaterCurrent = advancedpower.GetHeaterCurrent();

    // Input current
    inputCurrent = advancedpower.GetInputCurrent();

    #if HAS_MMU2()

    // MMU current
    mmuCurrent = advancedpower.GetMMUInputCurrent();

    #endif

#elif BOARD_IS_XLBUDDY()

    // Input voltage
    inputVoltage = advancedpower.Get24VVoltage();

    // Sandwich 5V voltage
    sandwich5VVoltage = advancedpower.Get5VVoltage();

    // Sandwich 5V current
    sandwich5VCurrent = advancedpower.GetDwarfSandwitch5VCurrent();

    // XLBUDDY board 5V current
    buddy5VCurrent = advancedpower.GetXLBuddy5VCurrent();

    buddy::puppies::Dwarf &dwarf = prusa_toolchanger.getActiveToolOrFirst();

    // Dwarf board temperature
    dwarfBoardTemperature = dwarf.get_board_temperature();

    // Dwarf MCU temperature
    dwarfMCUTemperature = dwarf.get_mcu_temperature();

#endif
}
