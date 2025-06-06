/// @file
#pragma once

#include "mmu2_state.h"
#include "mmu2_marlin.h"
#include "mmu2_reporting.h"
#include "mmu2_command_guard.h"
#include "mmu2_bootloader_result.h"

#ifdef __AVR__
    #include "mmu2_protocol_logic.h"
typedef float feedRate_t;

#else
    #include "protocol_logic.h"
    #include <atomic>
    #include <memory>
    #include <option/has_mmu2_over_uart.h>
#endif

struct E_Step;

#ifdef UNITTEST
    // Unit tests - avoid talking to the bootloader on the serial
    #define MMU_USE_BOOTLOADER() 0
#elif HAS_MMU2_OVER_UART()
    // Only MK4 with MMU over UART may try to talk to the bootloader on the serial
    #define MMU_USE_BOOTLOADER() 1
#else
    // CORE One with MMU over MODBUS - avoid talking to the bootloader on the serial
    #define MMU_USE_BOOTLOADER() 0
#endif

namespace MMU2 {

// general MMU setup for MK3
enum : uint8_t {
    FILAMENT_UNKNOWN = 0xffU
};

struct Version {
    uint8_t major, minor, build;
};

class MMU2BootloaderManager;

/// Top-level interface between Logic and Marlin.
/// Intentionally named MMU2 to be (almost) a drop-in replacement for the previous implementation.
/// Most of the public methods share the original naming convention as well.
class MMU2 {
public:
    MMU2();
    ~MMU2();

    /// Powers ON the MMU, then initializes the UART and protocol logic
    void Start();

    /// Stops the protocol logic, closes the UART, powers OFF the MMU
    void Stop();

    inline xState State() const { return state; }

    inline bool Enabled() const { return State() == xState::Active; }

    /// Different levels of resetting the MMU
    enum ResetForm : uint8_t {
        Software = 0, ///< sends a X0 command into the MMU, the MMU will watchdog-reset itself
        ResetPin = 1, ///< trigger the reset pin of the MMU
        CutThePower = 2, ///< power off and power on (that includes +5V and +24V power lines)
        EraseEEPROM = 42, ///< erase MMU EEPROM and then perform a software reset
    };

    /// Saved print state on error.
    enum SavedState : uint8_t {
        None = 0, // No state saved.
        ParkExtruder = 1, // The extruder was parked.
        Cooldown = 2, // The extruder was allowed to cool.
        CooldownPending = 4,
    };

    /// Tune value in MMU registers as a way to recover from errors
    /// e.g. Idler Stallguard threshold
    void Tune();

    /// Perform a reset of the MMU
    /// @param level physical form of the reset
    void Reset(ResetForm level);

    /// Power off the MMU (cut the power)
    void PowerOff();

    /// Power on the MMU
    void PowerOn();

    /// Returns result of the last MMU bootloader run
    inline MMU2BootloaderResult bootloader_result() const {
        return bootloader_result_;
    }

    /// Read from a MMU register (See gcode M707)
    /// @param address Address of register in hexidecimal
    /// @returns true upon success
    bool ReadRegister(uint8_t address);

    /// Write from a MMU register (See gcode M708)
    /// @param address Address of register in hexidecimal
    /// @param data Data to write to register
    /// @returns true upon success
    bool WriteRegister(uint8_t address, uint16_t data);

    /// The main loop of MMU processing.
    /// Doesn't loop (block) inside, performs just one step of logic state machines.
    /// Also, internally it prevents recursive entries.
    void mmu_loop();

    /// The main MMU command - select a different slot
    /// @param slot of the slot to be selected
    /// @returns false if the operation cannot be performed (Stopped)
    bool tool_change(uint8_t slot);

    /// Tool change that unloads the filament all the way from the nozzle and
    /// loads it also all the way into the nozzle. The "normal" toolchange
    /// relies on gcode to do the ramming and a load-to-nozzle sequence.
    ///
    /// It also parks the nozzle before unloading and unparks after loading the
    /// new filament. Used e.g. during spooljoin, which can't have gcode
    /// support and just has to do the change with minimal impact on the print.
    ///
    /// @param slot of the slot to be selected
    /// @returns false if the operation cannot be performed (Stopped)
    bool tool_change_full(uint8_t slot);

    /// Handling of special Tx, Tc, T? commands
    bool tool_change(char code, uint8_t slot);

    /// Unload of filament in collaboration with the MMU.
    /// That includes rotating the printer's extruder in order to release filament.
    /// @returns false if the operation cannot be performed (Stopped or cold extruder)
    bool unload();

    /// Load (insert) filament just into the MMU (not into printer's nozzle)
    /// @returns false if the operation cannot be performed (Stopped)
    bool load_filament(uint8_t slot);

    /// Load (push) filament from the MMU into the printer's nozzle
    /// @returns false if the operation cannot be performed (Stopped or cold extruder)
    bool load_filament_to_nozzle(uint8_t slot);

    /// Move MMU's selector aside and push the selected filament forward.
    /// Usable for improving filament's tip or pulling the remaining piece of filament out completely.
    bool eject_filament(uint8_t slot, bool enableFullScreenMsg = true);

    /// Issue a Cut command into the MMU
    /// Requires unloaded filament from the printer (obviously)
    /// @returns false if the operation cannot be performed (Stopped)
    bool cut_filament(uint8_t slot, bool enableFullScreenMsg = true);

    /// Issue a planned request for statistics data from MMU
    void get_statistics();

    /// Issue a Try-Load command
    /// It behaves very similarly like a ToolChange, but it doesn't load the filament
    /// all the way down to the nozzle. The sole purpose of this operation
    /// is to check, that the filament will be ready for printing.
    /// @param slot index of slot to be tested
    /// @returns true
    bool loading_test(uint8_t slot);

    /// @returns the active filament slot index (0-4) or 0xff in case of no active tool
    uint8_t get_current_tool() const;

    /// @returns The filament slot index (0 to 4) that will be loaded next, 0xff in case of no active tool change
    uint8_t get_tool_change_tool() const;

    bool set_filament_type(uint8_t slot, uint8_t type);

    /// Issue a "button" click into the MMU - to be used from Error screens of the MMU
    /// to select one of the 3 possible options to resolve the issue
    void Button(uint8_t index);

    /// Issue an explicit "homing" command into the MMU
    void Home(uint8_t mode);

    /// @returns current state of FINDA (true=filament present, false=filament not present)
    inline bool FindaDetectsFilament() const { return logic.FindaPressed(); }

    /// @returns current selector slot as reported by the MMU
    inline bool SelectorSlot() const { return logic.SelectorSlot(); }

    inline uint16_t TotalFailStatistics() const { return logic.FailStatistics(); }

    /// @returns Current error code
    inline ErrorCode MMUCurrentErrorCode() const { return logic.Error(); }

    /// @returns Last error source
    inline ErrorSource MMULastErrorSource() const { return lastErrorSource; }

    /// @returns the version of the connected MMU FW.
    /// In the future we'll return the trully detected FW version
    Version GetMMUFWVersion() const {
        if (State() == xState::Active) {
            return { logic.MmuFwVersionMajor(), logic.MmuFwVersionMinor(), logic.MmuFwVersionRevision() };
        } else {
            return { 0, 0, 0 };
        }
    }

    /// Method to read-only mmu_print_saved
    inline bool MMU_PRINT_SAVED() const { return mmu_print_saved != SavedState::None; }

    /// Automagically "press" a Retry button if we have any retry attempts left
    /// @param ec ErrorCode enum value
    /// @returns true if auto-retry is ongoing, false when retry is unavailable or retry attempts are all used up
    bool RetryIfPossible(ErrorCode ec);

    /// @return count for toolchange in current print
    inline uint16_t ToolChangeCounter() const { return toolchange_counter; };

    /// Set toolchange counter to zero
    inline void ClearToolChangeCounter() { toolchange_counter = 0; };

    inline uint16_t TMCFailures() const { return tmcFailures; }
    inline void IncrementTMCFailures() { ++tmcFailures; }
    inline void ClearTMCFailures() { tmcFailures = 0; }

    /// Retrieve cached value parsed from ReadRegister()
    /// or using M707
    inline uint16_t GetLastReadRegisterValue() const {
        return lastReadRegisterValue;
    };
    inline void InvokeErrorScreen(ErrorCode ec) {
        // The printer may not raise an error when the MMU is busy
        if (!logic.CommandInProgress() // MMU must not be busy
            && MMUCurrentErrorCode() == ErrorCode::OK // The protocol must not be in error state
            && lastErrorCode != ec) // The error code is not a duplicate
        {
            ReportError(ec, ErrorSource::ErrorSourcePrinter);
        }
    }

    void ClearPrinterError() {
        logic.ClearPrinterError();
        lastErrorCode = ErrorCode::OK;
        lastErrorSource = ErrorSource::ErrorSourceNone;
    }

    /// @brief Queue a button operation which the printer can act upon
    /// @param btn Button operation
    inline void SetPrinterButtonOperation(Buttons btn) {
        printerButtonOperation = btn;
    }

    /// @brief Get the printer button operation
    /// @return currently set printer button operation, it can be NoButton if nothing is queued
    inline Buttons GetPrinterButtonOperation() {
        return printerButtonOperation;
    }

    inline void ClearPrinterButtonOperation() {
        printerButtonOperation = Buttons::NoButton;
    }

    CommandInProgressManager commandInProgressManager;

#ifndef UNITTEST
private:
#endif
    /// Perform software self-reset of the MMU (sends an X0 command)
    void ResetX0();

    /// Perform software self-reset of the MMU + erase its EEPROM (sends X2a command)
    void ResetX42();

    /// Trigger reset pin of the MMU
    void TriggerResetPin();

    /// Perform power cycle of the MMU (cold boot)
    /// Please note this is a blocking operation (sleeps for some time inside while doing the power cycle)
    void PowerCycle();

    /// Stop the communication, but keep the MMU powered on (for scenarios with incorrect FW version)
    void StopKeepPowered();

    /// Along with the mmu_loop method, this loops until a response from the MMU is received and acts upon.
    /// In case of an error, it parks the print head and turns off nozzle heating
    /// @returns false if the command could not have been completed (MMU interrupted)
    [[nodiscard]] bool manage_response(const bool move_axes, const bool turn_off_nozzle);

    /// The inner private implementation of mmu_loop()
    /// which is NOT (!!!) recursion-guarded. Use caution - but we do need it during waiting for hotend resume to keep comms alive!
    /// @param reportErrors true if Errors should raise MMU Error screen, false otherwise
    void mmu_loop_inner(bool reportErrors);

    /// Performs one step of the protocol logic state machine
    /// and reports progress and errors if needed to attached ExtUIs.
    /// Updates the global state of MMU (Active/Connecting/Stopped) at runtime, see @ref State
    /// @param reportErrors true if Errors should raise MMU Error screen, false otherwise
    StepStatus LogicStep(bool reportErrors);

    void filament_ramming();

    /// If \p progressCode is set, reports executing the sequence
    void execute_extruder_sequence(const E_Step *sequence, uint8_t stepCount, ExtendedProgressCode progressCode = ExtendedProgressCode::_cnt);

    void execute_load_to_nozzle_sequence();

    /// Reports an error into attached ExtUIs
    /// @param ec error code, see ErrorCode
    /// @param res reporter error source, is either Printer (0) or MMU (1)
    void ReportError(ErrorCode ec, ErrorSource res);

    /// Reports progress of operations into attached ExtUIs
    /// @param pc progress code, see ProgressCode
    void ReportProgress(ProgressCode pc);

    /// Responds to a change of MMU's progress
    /// - plans additional steps, e.g. starts the E-motor after fsensor trigger
    /// The function is quite complex, because it needs to handle asynchronnous
    /// progress and error reports coming from the MMU without an explicit command
    /// - typically after MMU's start or after some HW issue on the MMU.
    /// It must ensure, that calls to @ref ReportProgress and/or @ref ReportError are
    /// only executed after @ref BeginReport has been called first.
    void OnMMUProgressMsg(ProgressCode pc);
    /// Progress code changed - act accordingly
    void OnMMUProgressMsgChanged(ProgressCode pc);
    /// Repeated calls when progress code remains the same
    void OnMMUProgressMsgSame(ProgressCode pc);

    /// @brief Save hotend temperature and set flag to cooldown hotend after 60 minutes
    /// @param turn_off_nozzle if true, the hotend temperature will be set to 0degC after 60 minutes
    void SaveHotendTemp(bool turn_off_nozzle);

    /// Save print and park the print head
    void SaveAndPark(bool move_axes);

    /// Resume hotend temperature, if it was cooled. Safe to call if we aren't saved.
    void ResumeHotendTemp();

    /// Resume position, if the extruder was parked. Safe to all if state was not saved.
    void ResumeUnpark();

    /// Check for any button/user input coming from the printer's UI
    void CheckUserInput();

    /// @brief Check whether to trigger a FINDA runout. If triggered this function will call M600 AUTO
    /// if SpoolJoin is enabled, otherwise M600 is called without AUTO which will prompt the user
    /// for the next filament slot to use
    void CheckFINDARunout();

    /// Entry check of all external commands.
    /// It can wait until the MMU becomes ready.
    /// Optionally, it can also emit/display an error screen and the user can decide what to do next.
    /// @returns false if the MMU is not ready to perform the command (for whatever reason)
    bool WaitForMMUReady();

    /// Generic testing procedure if filament entered the extruder (PTFE or nube).
    /// @returns false if test fails, true otherwise
    bool VerifyFilamentEnteredPTFE();
    /// After MMU completes a tool-change command
    /// the printer will push the filament by a constant distance. If the Fsensor untriggers
    /// at any moment the test fails. Else the test passes, and the E-motor retracts the
    /// filament back to its original position.
    /// @returns false if test fails, true otherwise
    bool TryLoad();
    /// MK4 doesn't need to perform a try-load - it can leverage the LoadCell to detect vibrations of the E-motor in case the filament gets stuck.
    /// Using this procedure is faster and causes less wear of the filament.
    /// @returns false if test fails, true otherwise
    bool FeedWithEStallDetection();
    /// experimental procedure to perform "try-loads" at different speeds - not usable for printing,
    /// but important for tuning of the EStall detection while feeding filament into the nube.
    /// @returns false if test fails, true otherwise
    bool MeasureEStallAtDifferentSpeeds();

    /// Common processing of pushing filament into the extruder - shared by tool_change, load_to_nozzle and probably others
    void ToolChangeCommon(uint8_t slot);
    bool ToolChangeCommonOnce(uint8_t slot);

    void HelpUnloadToFinda();

    enum class PreUnloadPolicy {
        Nothing,
        Ramming,
        RelieveFilament,
        ExtraRelieveFilament, // longer retraction for E-stall enabled printers
    };
    void UnloadInner(PreUnloadPolicy preUnloadPolicy);
    void CutFilamentInner(uint8_t slot);

    ProtocolLogic logic; ///< implementation of the protocol logic layer

#if MMU_USE_BOOTLOADER()
    std::unique_ptr<MMU2BootloaderManager> bootloader; ///< Bootloader manager, handles firmware updates and such
#endif
    std::atomic<MMU2BootloaderResult> bootloader_result_ = MMU2BootloaderResult::not_detected;

    uint8_t extruder; ///< currently active slot in the MMU ... somewhat... not sure where to get it from yet
    uint8_t tool_change_extruder; ///< only used for UI purposes

    pos3d resume_position;
    int16_t resume_hotend_temp;

    ProgressCode lastProgressCode = ProgressCode::OK;
    ErrorCode lastErrorCode = ErrorCode::MMU_NOT_RESPONDING;
    ErrorSource lastErrorSource = ErrorSource::ErrorSourceNone;
    Buttons lastButton = Buttons::NoButton;
    uint16_t lastReadRegisterValue = 0;
    Buttons printerButtonOperation = Buttons::NoButton;

    StepStatus logicStepLastStatus;

    std::atomic<xState> state;

    uint8_t mmu_print_saved;
    bool loadFilamentStarted;
    bool unloadFilamentStarted;

    uint16_t toolchange_counter;
    uint16_t tmcFailures;

    /// MMU originated CIP reports have a custom guard - this variable holds whether we have called incGuard for that case
    bool mmuOriginatedCommandGuard = false;

    /// Default nominal unload fsensor trigger distance.
    /// As this largely depends on the slicer profile, a register for setting a custom value is provided along with it.
    static constexpr float nominalEMotorFSOff = 14.0F;
    /// The "register" allowing user tweaking of the nominal trigger distance. Set a custom value through M708 A0x81 Xsomething.
    float nominalEMotorFSOffReg = nominalEMotorFSOff;
    /// A record of the last E-motor position when fsensor turned off while unloading
    float unloadEPosOnFSOff = nominalEMotorFSOff;
    /// register - fail the next n loads to extr intentionally
    /// write a nonzero value into this register, it will screw up the load returned value (even if it went perfectly smooth on the machine)
    /// the register is decremented after each load to extr. automatically
    uint8_t failNextLoadToExtr = 0;

    bool CheckFailLoadToExtr(bool b);
};

/// following Marlin's way of doing stuff - one and only instance of MMU implementation in the code base
/// + avoiding buggy singletons on the AVR platform
extern MMU2 mmu2;

} // namespace MMU2
