/// @file
#include "automatic_chamber_vents.hpp"

#include <feature/print_status_message/print_status_message_guard.hpp>
#include <mapi/parking.hpp>
#include <Marlin/src/gcode/gcode.h>
#include <Marlin/src/module/motion.h>
#include <Marlin/src/module/planner.h>
#include <printers.h>

#include <feature/chamber/chamber.hpp>

namespace automatic_chamber_vents {
namespace {

    /// The following constants define key positions for controlling a vent lever.
    /// These coordinates are absolute in the printer's coordinate system.
#if PRINTER_IS_PRUSA_COREONE()
    static constexpr auto Y_SAFE = -3.f; ///< Safe Y line with no risk of coming in contact with lever
    static constexpr auto Y_LEVER = -18.f; ///< In line with the lever
    static constexpr auto X_OPEN_START_POS = 37.f; ///< An X-axis position to the right of the lever, where opening move starts.
    static constexpr auto X_OPEN_END_POS = 24.f; ///< The X-axis position to move to on Y_LEVER to open the vents.
    static constexpr auto X_CLOSE_START_POS = 11.f; ///< An X-axis position to the left of the lever, where closing move starts.
    static constexpr auto X_CLOSE_END_POS = 26.f; ///< The X-axis position to move to on Y_LEVER to close the vents.
    static constexpr auto X_LEVER_MOVE_AWAY = 4.f; ///< The X-axis distance to move away from the lever after switch
    static constexpr auto lever_move_feedrate = feedRate_t(200.0f);
#else
    #error
#endif

    enum class VentState {
        open,
        close
    };

    /// @brief Plans a move to a new X-axis coordinate.
    /// @param x The target X-axis position.
    /// @param feedrate The speed of the move in mm/s.
    void plan_to_x(float x, feedRate_t feedrate = feedRate_t(XY_PROBE_FEEDRATE_MM_S)) {
        xyze_pos_t xyz = current_position;
        xyz.x = x;
        destination = xyz;
        prepare_internal_move_to_destination(feedrate, { .apply_modifiers = false /*XY move doesn't need MBL*/ });
    }

    /// @brief Plans a move to a new Y-axis coordinate.
    /// @param y The target Y-axis position.
    /// @param feedrate The speed of the move in mm/s.
    void plan_to_y(float y, feedRate_t feedrate = feedRate_t(XY_PROBE_FEEDRATE_MM_S)) {
        xyze_pos_t xyz = current_position;
        xyz.y = y;
        destination = xyz;
        prepare_internal_move_to_destination(feedrate, { .apply_modifiers = false /*XY move doesn't need MBL*/ });
    }

    /// @brief Prepares the printer for a vent lever switch.
    /// @return true on success, false on failure.
    bool before() {
        if (!GcodeSuite::G28_no_parser(true, true, false, { .only_if_needed = true, .precise = false })) {
            return false;
        }
        return true;
    }

    void after() {
        // Return to the home position after the vent operation.
        mapi::park(mapi::ZAction::no_move, mapi::park_positions[mapi::ParkPosition::park]);
    }

    void switch_lever(VentState wanted_state) {
        // Move to a safe Y-axis position to avoid the lever.
        plan_to_y(Y_SAFE, lever_move_feedrate);
        // Move to a horizontal position (left or right) of the lever.
        plan_to_x(wanted_state == VentState::open ? X_OPEN_START_POS : X_CLOSE_START_POS, lever_move_feedrate);
        // Move into the lever's Y-axis line.
        plan_to_y(Y_LEVER, lever_move_feedrate);
        // Move horizontally to engage the lever and switch it.
        plan_to_x(wanted_state == VentState::open ? X_OPEN_END_POS : X_CLOSE_END_POS, lever_move_feedrate);
        // Move horizontally to release the lever tension
        plan_to_x(wanted_state == VentState::open ? X_OPEN_END_POS + X_LEVER_MOVE_AWAY : X_CLOSE_END_POS - X_LEVER_MOVE_AWAY, lever_move_feedrate);
        // Back out to the safe Y-axis position to avoid a collision on future moves.
        plan_to_y(Y_SAFE, lever_move_feedrate);
    }

}; // namespace

bool open() {
    PrintStatusMessageGuard psm_guard;
    psm_guard.update<PrintStatusMessage::Type::opening_chamber_vents>({});

    if (!before()) {
        return false;
    }
    switch_lever(VentState::open);
    buddy::chamber().set_vent_state(buddy::Chamber::VentState::open);
    after();
    planner.synchronize(); // Wait for all planned moves to complete
    return true;
}

bool close() {
    PrintStatusMessageGuard psm_guard;
    psm_guard.update<PrintStatusMessage::Type::closing_chamber_vents>({});

    if (!before()) {
        return false;
    }

    switch_lever(VentState::close);
    buddy::chamber().set_vent_state(buddy::Chamber::VentState::closed);
    after();
    planner.synchronize(); // Wait for all planned moves to complete
    return true;
}
} // namespace automatic_chamber_vents
