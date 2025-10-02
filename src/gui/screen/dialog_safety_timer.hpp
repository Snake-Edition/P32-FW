/// @file
#pragma once

#include <screen_fsm.hpp>

class DialogSafetyTimer : public DialogFSM {
public:
    DialogSafetyTimer(fsm::BaseData data);
    ~DialogSafetyTimer();

protected:
    void create_frame() override;
    void destroy_frame() override;
    void update_frame() override;
};
