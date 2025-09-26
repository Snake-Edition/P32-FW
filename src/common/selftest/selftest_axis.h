// selftest_axis.h
#pragma once

#include <inttypes.h>
#include "i_selftest_part.hpp"
#include "selftest_loop_result.hpp"
#include "selftest_axis_config.hpp"
#include "selftest_log.hpp"

#include <printers.h>
namespace selftest {

class CSelftestPart_Axis {
    IPartHandler &state_machine;
    const AxisConfig_t &config;
    SelftestSingleAxis_t &rResult;
    uint32_t time_progress_start = 0;
    uint32_t time_progress_estimated_end = 0;
    uint32_t m_StartPos_usteps = 0;
    uint8_t m_Step = 0;
#if !PRINTER_IS_PRUSA_XL()
    float unmeasured_distance = 0; // Distance traveled before axis measuring is started
#endif
    bool coils_ok = false; // Initially false, set to true when any coil check passes

    void phaseMove(int8_t dir);
    LoopResult wait(int8_t dir);
    static uint32_t estimate(const AxisConfig_t &config);
    static uint32_t estimate_move(float len_mm, float fr_mms);
    void actualizeProgress() const;
    LogTimer log;
    int getDir() { return (m_Step % 2) ? -config.movement_dir : config.movement_dir; }

    enum class Motor { stp_400,
        stp_200 };
    static void motor_switch(Motor steps);

public:
    using Config = AxisConfig_t;

    static constexpr float EXTRA_LEN_MM = 10; // How far to move behind expected axis end

    CSelftestPart_Axis(IPartHandler &state_machine, const AxisConfig_t &config,
        SelftestSingleAxis_t &result);
    ~CSelftestPart_Axis();

    LoopResult stateActivateHomingReporter();
    LoopResult stateHomeXY(); ///< Enqueue homing
    LoopResult stateWaitHomingReporter(); ///< Alternative state to stateWaitHome, in case reporter is used
    LoopResult stateEvaluateHomingXY();

    LoopResult stateHomeZ(); ///< Enqueue homing and toolchange
    LoopResult stateWaitHome(); ///< Wait for homing and toolchange to finish

    LoopResult stateInitProgressTimeCalculation();
    LoopResult stateCycleMark2() { return LoopResult::MarkLoop2; }
    LoopResult stateMove();
    LoopResult stateMoveFinishCycle();
    LoopResult stateParkAxis();
    LoopResult state_verify_coils(); ///< Report error when coils were never seen ok

private:
    /**
     * \brief Check stepper coils for open/short circuit
     * Needs to be called relatively during move to cope with false error detection.
     * A single passing check sets coils_ok to true.
     */
    void check_coils();
};

extern const AxisConfig_t Config_XAxis;
extern const AxisConfig_t Config_YAxis;

}; // namespace selftest
