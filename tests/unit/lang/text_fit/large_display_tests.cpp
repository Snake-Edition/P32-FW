#include <catch2/catch.hpp>
#include "errors.hpp"
#include "text_fit_test.hpp"

TEST_CASE("Error yaml text fit [MINI display]") { // "[!mayfail]"
    // Error texts from YAML file are extracted by utils/extract_error_texts.py (see CMakeLists.txt)
    for (auto &error : error_list) {
        if (error.display == DisplayOption::mini) {
            continue;
        }
        test_error(error);
    }
}
