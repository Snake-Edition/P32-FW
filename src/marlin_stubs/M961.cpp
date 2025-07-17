#include "PrusaGcodeSuite.hpp"

#include <Marlin/src/Marlin.h>
#include <Marlin/src/gcode/calibrate/M958.hpp>
#include <feature/motordriver_util.h>
#include <feature/phase_stepping/phase_stepping.hpp>
#include <gcode/queue.h> // For pause serial

#include <option/has_xbuddy_extension.h>

#if HAS_XBUDDY_EXTENSION()
    #include <feature/xbuddy_extension/xbuddy_extension.hpp>
#endif

/**
 * Manual belt tuning.
 *
 * For now, this is a placeholder which reads instructions from serial line.
 * This will change to an interactive wizard.
 */
void PrusaGcodeSuite::M961() {
    // phstep needs to be off _before_ getting the current ustep resolution
    phase_stepping::EnsureDisabled phaseSteppingDisabler;
    MicrostepRestorer microstepRestorer;

    Vibrate vibrator {
        .frequency = 80,
        .excitation_acceleration = 2.5f,
        .axis_flag = STEP_EVENT_FLAG_STEP_X | STEP_EVENT_FLAG_STEP_Y | STEP_EVENT_FLAG_Y_DIR, // Vibrate the toolhead front and back
    };

    // Taken from M958::setup_axis
    // enable all axes to have the same state as printing
    enable_all_steppers();
    stepper_microsteps(X_AXIS, 128);
    stepper_microsteps(Y_AXIS, 128);

    if (!vibrator.setup(microstepRestorer)) {
        return;
    }

    bool keep_going = true;
    constexpr size_t bufsize = 80;
    char buffer[80];
    strcpy(buffer, "M961 ");
    size_t buffer_pos = 5;
    GCodeQueue::pause_serial_commands = true;
    while (keep_going) {
        vibrator.step();
        while (SerialUSB.available()) {
            buffer[buffer_pos++] = SerialUSB.read();
            if (buffer_pos == bufsize || buffer[buffer_pos - 1] == '\r') {
                GCodeParser2 parser;
                if (parser.parse(std::string_view(buffer, buffer + buffer_pos - 1 /* Avoid the \r */))) {
                    bool changed = false;
                    if (parser.has_option('Q')) {
                        keep_going = false;
                    }
                    if (parser.store_option('F', vibrator.frequency)) {
                        vibrator.frequency = abs(vibrator.frequency);
                        changed = true;
                    }
                    if (parser.store_option('A', vibrator.excitation_acceleration)) {
                        vibrator.excitation_acceleration = abs(vibrator.excitation_acceleration);
                        changed = true;
                    }
#if HAS_XBUDDY_EXTENSION()
                    uint16_t strobe_freq = 0;
                    if (parser.store_option('T', strobe_freq)) {
                        buddy::xbuddy_extension().set_strobe(strobe_freq);
                        changed = true;
                    }
#endif
                    if (changed) {
                        SERIAL_ECHO("Changed params");
                    }
                } else {
                    SERIAL_ECHO("Parse error");
                }
                buffer_pos = 5;
            }
        }
    }
#if HAS_XBUDDY_EXTENSION()
    buddy::xbuddy_extension().set_strobe(std::nullopt);
#endif
    GCodeQueue::pause_serial_commands = false;
    SERIAL_ECHO("Quit");
}
