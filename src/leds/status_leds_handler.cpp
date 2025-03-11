#include "leds/status_leds_handler.hpp"

#include "leds/animation_controller.hpp"
#include "leds/frame_animation.hpp"
#include "marlin_vars.hpp"

namespace leds {

using namespace marlin_server;

static StateAnimation marlin_to_anim_state() {
    const marlin_server::State printer_state = marlin_vars().print_state;

    switch (printer_state) {
    case State::Idle:
    case State::WaitGui:
    case State::PrintPreviewInit:
    case State::PrintPreviewImage:
    case State::PrintPreviewConfirmed:
    case State::PrintPreviewQuestions:
#if HAS_TOOLCHANGER() || HAS_MMU2()
    case State::PrintPreviewToolsMapping:
#endif
    case State::Exit:
        return StateAnimation::Idle;

    case State::Printing:
    case State::PrintInit:
    case State::SerialPrintInit:
    case State::Finishing_WaitIdle:
    case State::Pausing_Begin:
    case State::Pausing_Failed_Code:
    case State::Pausing_WaitIdle:
    case State::Pausing_ParkHead:
    case State::Resuming_BufferData:
    case State::Resuming_Begin:
    case State::Resuming_Reheating:
    case State::Resuming_UnparkHead_XY:
    case State::Resuming_UnparkHead_ZE: {
        auto fsm_states = marlin_vars().get_fsm_states();
        if (fsm_states.is_active(ClientFSM::Load_unload) || fsm_states.is_active(ClientFSM::Preheat)) {
            return StateAnimation::Warning;
        } else {
            return StateAnimation::Printing;
        }
    }
    case State::Paused:
        return StateAnimation::Warning;

    case State::Aborting_Begin:
    case State::Aborting_WaitIdle:
    case State::Aborting_ParkHead:
    case State::Aborting_Preview:
    case State::Aborting_UnloadFilament:
    case State::Aborted:
        return StateAnimation::Aborting;

    case State::Finishing_ParkHead:
    case State::Finishing_UnloadFilament:
    case State::Finished:
        return StateAnimation::Finishing;

    case State::CrashRecovery_Begin:
    case State::CrashRecovery_Retracting:
    case State::CrashRecovery_Lifting:
    case State::CrashRecovery_ToolchangePowerPanic:
    case State::CrashRecovery_XY_Measure:
#if HAS_TOOLCHANGER()
    case State::CrashRecovery_Tool_Pickup:
#endif
    case State::CrashRecovery_XY_HOME:
    case State::CrashRecovery_HOMEFAIL:
    case State::CrashRecovery_Axis_NOK:
    case State::CrashRecovery_Repeated_Crash:
        return StateAnimation::Warning;

    case State::PowerPanic_acFault:
    case State::PowerPanic_Resume:
    case State::PowerPanic_AwaitingResume:
        return StateAnimation::PowerPanic;
    }
    return StateAnimation::Idle;
}

namespace {

    using StateAnimationController = AnimationController<FrameAnimation, StateAnimation, 3>;

    constexpr auto solid = std::to_array<FrameAnimation<3>::Frame>({ { 100, 100, 100 } });

    constexpr auto pulsing = std::to_array<FrameAnimation<3>::Frame>({ { 100, 100, 100 },
        { 0, 0, 0 } });

    constexpr StateAnimationController::Mapping animations {
        { StateAnimation::Idle, { { 0, 0, 0 }, 1000, 0, 400, solid } },
        { StateAnimation::Printing, { { 0, 150, 255 }, 1000, 0, 400, solid } },
        { StateAnimation::Aborting, { { 0, 0, 0 }, 1000, 0, 400, solid } },
        { StateAnimation::Finishing, { { 0, 255, 0 }, 1000, 0, 400, solid } },
        { StateAnimation::Warning, { { 255, 255, 0 }, 1000, 0, 1000, pulsing } },
        { StateAnimation::PowerPanic, { { 0, 0, 0 }, 1000, 0, 400, solid } },
        { StateAnimation::PowerUp, { { 0, 255, 0 }, 1500, 0, 1500, pulsing } },
        { StateAnimation::Error, { { 255, 0, 0 }, 500, 0, 500, pulsing } },
        { StateAnimation::Custom, { { 0, 0, 0 }, 1000, 0, 300, solid } },
    };

} // namespace

StateAnimationController &controller_instance() {
    static StateAnimationController instance { animations, StateAnimation::Idle, StateAnimation::Custom };
    return instance;
}

StatusLedsHandler &StatusLedsHandler::instance() {
    static StatusLedsHandler instance;
    return instance;
}

void StatusLedsHandler::set_error() {
    std::lock_guard lock(mutex);
    is_error_state = true;
}

void StatusLedsHandler::set_animation(StateAnimation state) {
    std::lock_guard lock(mutex);
    controller_instance().set(state);
}

bool StatusLedsHandler::get_active() {
    std::lock_guard lock(mutex);
    return active;
}

void StatusLedsHandler::set_custom_animation(const ColorRGBW &color, AnimationType type, uint16_t period_ms) {
    std::lock_guard lock(mutex);
    auto &controller = controller_instance();
    auto &custom_params = controller.get_custom_params();

    custom_params.color = color;
    switch (type) {
    case AnimationType::Solid:
        custom_params.frames = solid;
        break;
    case AnimationType::Pulsing:
        custom_params.frames = pulsing;
        break;
    }

    if (period_ms > 0) {
        custom_params.frame_length = period_ms / custom_params.frames.size();
        custom_params.blend_time = custom_params.frame_length / 4;
    } else {
        custom_params.frame_length = 1000;
        custom_params.blend_time = 300;
    }

    controller.set(StateAnimation::Custom);
}

void StatusLedsHandler::set_active(bool val) {
    std::lock_guard lock(mutex);
    active = val;
    config_store().run_leds.set(val);
}

void StatusLedsHandler::update() {
    std::lock_guard lock(mutex);

    StateAnimation state;
    if (!active) {
        state = StateAnimation::Idle; // assuming LEDs are off in Idle
    } else if (is_error_state) {
        state = StateAnimation::Error;
    } else {
        state = marlin_to_anim_state();
    }

    if (state != old_state) {
        old_state = state;
        controller_instance().set(state);
    }

    controller_instance().update();
}

std::span<const ColorRGBW, 3> StatusLedsHandler::led_data() {
    std::lock_guard lock(mutex);
    return controller_instance().data();
}

} // namespace leds
