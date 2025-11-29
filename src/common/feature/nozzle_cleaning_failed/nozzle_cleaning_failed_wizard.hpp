/// @file
#pragma once

namespace nozzle_cleaning_failed_wizard {

enum class Result {
    abort, // abort print
    ignore, // continute without retrying
    retry, // retry nozzle cleaning
};

/**
 * @brief Run the nozzle cleaning failed wizard.
 * @return Result of the wizard.
 */
Result run_wizard();

} // namespace nozzle_cleaning_failed_wizard
