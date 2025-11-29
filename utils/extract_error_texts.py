import yaml
import os
import sys

display_large = "DisplayOption::large"
display_all = "DisplayOption::all"
display_mini = "DisplayOption::mini"


def load_yaml(file_path):
    with open(file_path, 'r') as file:
        return yaml.safe_load(file)


def determine_display_option(error, is_mmu):
    printers = error.get("printers", [])
    if is_mmu:
        return display_large
    if len(printers) == 0:
        return display_all
    if "MINI" in printers:
        return display_all if len(printers) > 1 else display_mini
    return display_large


def process_errors(data, is_mmu=False):
    processed_errors = []
    error_list = data.get("Errors", [])
    if (len(error_list) == 0):
        raise ValueError("No errors found")

    for error in error_list:
        if "gui_layout" not in error:
            continue

        processed_errors.append({
            "title":
            error.get("title",
                      "").replace("\\",
                                  "\\\\").replace("\"",
                                                  "\\\"").replace("\n", "\\n"),
            "text":
            error.get("text",
                      "").replace("\\",
                                  "\\\\").replace("\"",
                                                  "\\\"").replace("\n", "\\n"),
            "display":
            determine_display_option(error, is_mmu),
            "gui_layout":
            "GuiLayout::" + error.get("gui_layout", "")
        })
    return processed_errors


def generate_header(errors, header_file):
    with open(header_file, 'w') as file:
        file.write("#pragma once\n\n")
        file.write("#include <cstddef>\n#include <array>\n\n")
        file.write("#include <cstddef>\n#include <text_fit_enums.hpp>\n\n")
        file.write(f"constexpr size_t ERROR_COUNT = {len(errors)};\n")
        file.write(
            "static constexpr std::array<ErrorEntry, ERROR_COUNT> error_list {\n"
        )
        for error in errors:
            file.write(
                f'    ErrorEntry("{error["title"]}", "{error["text"]}", {error["display"]}, {error["gui_layout"]}),\n'
            )
        file.write("};\n")


def main():
    if len(sys.argv) != 3:
        print(
            "Usage: python script.py <buddy-error-codes.yaml> <mmu-error-codes.yaml>"
        )
        sys.exit(1)

    buddy_yaml = sys.argv[1]
    mmu_yaml = sys.argv[2]

    buddy_data = load_yaml(buddy_yaml)
    mmu_data = load_yaml(mmu_yaml)

    processed_errors = process_errors(buddy_data) + process_errors(mmu_data,
                                                                   is_mmu=True)

    generate_header(processed_errors, "errors.hpp")
    print("Generated errors.hpp and successfully.")


if __name__ == "__main__":
    main()
