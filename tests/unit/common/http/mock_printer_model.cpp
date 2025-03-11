#include <common/printer_model.hpp>

static PrinterModelInfo printer_model_info {
    .model = PrinterModel::mk3,
    .compatibility_group = PrinterModelCompatibilityGroup::mk3,
    .version = { 1, 3, 0 },
    .help_url = nullptr,
    .usb_pid = 0,
    .gcode_check_code = 300,
    .id_str = "MK3",
};

const PrinterModelInfo &PrinterModelInfo::current() {
    return printer_model_info;
}
