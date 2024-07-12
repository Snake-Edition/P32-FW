// screen_mbl_values.hpp

#include "gui.hpp"
#include "window_text.hpp"
#include "window_numb.hpp"
#include "guitypes.hpp"
#include "screen.hpp"

class screen_mbl_values_t : public screen_t {
public:
    screen_mbl_values_t();
    ~screen_mbl_values_t() {}

protected:
    virtual void windowEvent(window_t *sender, GUI_event_t event, void *param) override;

private:
    virtual void draw() override;
};
