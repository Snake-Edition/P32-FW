#pragma once
#include <window_msgbox.hpp>

bool selftest_warning_selftest_finished();
void warn_unfinished_selftest_msgbox();

// Marks selftest as passed in config store and unblocks known gates (eg. sheet calibration)
void mark_selftest_as_passed_and_unblock();
