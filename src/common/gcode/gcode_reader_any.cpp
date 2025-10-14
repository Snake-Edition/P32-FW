#include "gcode_reader_any.hpp"

#include "lang/i18n.h"
#include "transfers/transfer.hpp"
#include <cassert>
#include <errno.h> // for EAGAIN
#include <filename_type.hpp>
#include <sys/stat.h>
#include <type_traits>

AnyGcodeFormatReader::~AnyGcodeFormatReader() {
}

AnyGcodeFormatReader::AnyGcodeFormatReader(AnyGcodeFormatReader &&other)
    : storage { std::move(other.storage) } {
    other.close();
}

AnyGcodeFormatReader &AnyGcodeFormatReader::operator=(AnyGcodeFormatReader &&other) {
    storage = std::move(other.storage);
    other.close();
    return *this;
}

AnyGcodeFormatReader::AnyGcodeFormatReader()
    : storage { ClosedReader {} } {
}

AnyGcodeFormatReader::AnyGcodeFormatReader(const char *filename)
    : storage { ClosedReader {} } {

    open(filename);
}

bool AnyGcodeFormatReader::open(const char *filename) {
    close();

    transfers::Transfer::Path path(filename);
    struct stat info {};

    // check if file is partially downloaded file
    bool is_partial = false;

    if (stat(path.as_destination(), &info) != 0) {
        return false;
    }

    if (S_ISDIR(info.st_mode)) {
        if (stat(path.as_partial(), &info) != 0) {
            return false;
        }

        is_partial = true;
    }

    FILE *file = fopen(is_partial ? path.as_partial() : path.as_destination(), "rb");
    if (!file) {
        return false;
    }

    if (filename_is_bgcode(filename)) {
        storage.emplace<PrusaPackGcodeReader>(*file, info);
    }

    else if (filename_is_plain_gcode(filename)) {
        storage.emplace<PlainGcodeReader>(*file, info);
    }

    else {
        fclose(file);
        return false;
    }

    if (is_partial) {
        get()->update_validity(path.as_destination());
    }

    return true;
}

void AnyGcodeFormatReader::close() {
    storage.emplace<ClosedReader>();
}

IGcodeReader *AnyGcodeFormatReader::get() {
    return std::visit([](auto &t) -> IGcodeReader * { return &t; }, storage);
}

bool AnyGcodeFormatReader::is_open() const {
    return !std::holds_alternative<ClosedReader>(storage);
}
