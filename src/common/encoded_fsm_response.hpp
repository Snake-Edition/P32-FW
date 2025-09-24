#pragma once

#include <client_response.hpp>
#include <common/primitive_any.hpp>

using FSMResponseVariant = PrimitiveAny<4>;

// This holds type-erased FSM response to reduce module dependencies
struct EncodedFSMResponse {
    FSMResponseVariant response;
    FSMAndPhase fsm_and_phase;
};

constexpr EncodedFSMResponse empty_encoded_fsm_response = {
    .response {},
    .fsm_and_phase { ClientFSM::_count, 0 },
};

#ifndef UNITTESTS
static_assert(sizeof(EncodedFSMResponse) <= 12);
#endif
