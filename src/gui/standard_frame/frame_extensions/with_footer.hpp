#pragma once

#include "footer_line.hpp"
#include "window_frame.hpp"

template <typename Base, FooterLine::IdArray footer_items>
class WithFooter : public Base {

public:
    static_assert(
        requires(Base b, FooterLine l) { b.add_footer(l); }, "Base frame needs to implement add_footer");

    template <typename... Args>
    WithFooter(window_frame_t *parent, Args &&...args)
        : Base(parent, std::forward<Args>(args)...)
        , footer(parent, 0) {
        footer.Create(footer_items);
        Base::add_footer(footer);
    }

protected:
    FooterLine footer;
};
