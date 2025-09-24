#pragma once

#include <encoded_fsm_response.hpp>
#include <freertos/queue.hpp>
#include <marlin_events.h>
#include <gcode/inject_queue_actions.hpp>
#include <warning_type.hpp>
#include <option/has_selftest.h>
#include <option/has_cancel_object.h>

namespace marlin_server {

/// marlin_client -> marlin_server request
struct Request {
    enum class Type : uint8_t {
        EventMask,
        Gcode,
        Inject,
        SetVariable,
        Babystep,
#if HAS_SELFTEST()
        TestStart,
#endif
        PrintStart,
        FSM,
        CancelObjectID,
        UncancelObjectID,
        SetWarning,
    };

    union {
        uint64_t event_mask = 0; // Type::EventMask
        int cancel_object_id; // Type::CancelObjectID/Type::UncancelObjectID
        struct {
            uintptr_t variable;
            union {
                float float_value;
                uint32_t uint32_value;
            };
        } set_variable; // Type::SetVariable
        struct {
            uint64_t test_mask;
            size_t test_data_index;
            uint32_t test_data_data;
        } test_start; // Type::TestStart
        char gcode[MARLIN_MAX_REQUEST + 1]; // Type::Gcode
        InjectQueueRecord inject; // Type::Inject
        EncodedFSMResponse encoded_fsm_response; // Type::FSM
        float babystep; // Type::Babystep
        struct {
            marlin_server::PreviewSkipIfAble skip_preview;
            char filename[FILE_PATH_BUFFER_LEN];
        } print_start; // Type::PrintStart
        WarningType warning_type;
    };

    /// if it is set to 1, then the marlin server sends an acknowledge (default)
    /// in some cases (sending a request from svc task) waiting is prohibited and it is necessary not to request an acknowledgment
    unsigned response_required : 1;
    unsigned client_id : 7;
    Type type;
};

using RequestQueue = freertos::Queue<Request, 1>;
extern RequestQueue request_queue;

enum class RequestFlag : uint8_t {
    PrintReady,
    PrintAbort,
    PrintPause,
    PrintResume,
    TryRecoverFromMediaError,
    PrintExit,
    KnobMoveUp,
    KnobMoveDown,
    KnobClick,
    GuiCantPrint,
#if HAS_SELFTEST()
    TestAbort,
#endif
#if HAS_CANCEL_OBJECT()
    CancelCurrentObject,
#endif
    _cnt
};

} // namespace marlin_server
