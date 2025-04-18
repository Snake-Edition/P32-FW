#include <version/version.hpp>

#include <algorithm>
#include <cstring>
#include <option/bootloader.h>
#include <option/enable_translation_cs.h>
#include <option/enable_translation_de.h>
#include <option/enable_translation_es.h>
#include <option/enable_translation_fr.h>
#include <option/enable_translation_it.h>
#include <option/enable_translation_ja.h>
#include <option/enable_translation_pl.h>
#include <printers.h>

#define _STR(x) #x
#define STR(x)  _STR(x)

namespace version {

const char project_version[] = STR(FW_VERSION);

constexpr const uint16_t project_version_major = FW_VERSION_MAJOR;
constexpr const uint16_t project_version_minor = FW_VERSION_MINOR;
constexpr const uint16_t project_version_patch = FW_VERSION_PATCH;

const char project_version_full[] = STR(FW_VERSION_FULL);

const char project_version_suffix[] = STR(FW_VERSION_SUFFIX);

const char project_version_suffix_short[] = STR(FW_VERSION_SUFFIX_SHORT);

const int project_build_number = FW_BUILD_NUMBER;

#if PRINTER_IS_PRUSA_MINI()
const char project_firmware_name[] = "Buddy_MINI";
#elif PRINTER_IS_PRUSA_XL()
const char project_firmware_name[] = "Buddy_XL";
#elif PRINTER_IS_PRUSA_MK4()
const char project_firmware_name[] = "Buddy_MK4";
#elif PRINTER_IS_PRUSA_MK3_5()
const char project_firmware_name[] = "Buddy_MK3_5";
#elif PRINTER_IS_PRUSA_iX()
const char project_firmware_name[] = "Buddy_iX";
#elif PRINTER_IS_PRUSA_COREONE()
const char project_firmware_name[] = "Buddy_CORE_ONE";
#else
    #error "Unknown PRINTER_TYPE."
#endif

const BuildIdentification project_build_identification {
    .commit_hash = STR(FW_COMMIT_HASH),
    .project_version_full = STR(FW_VERSION_FULL),
    .enabled_translations = (0 //
        | ENABLE_TRANSLATION_CS() << 0
        | ENABLE_TRANSLATION_DE() << 1
        | ENABLE_TRANSLATION_ES() << 2
        | ENABLE_TRANSLATION_FR() << 3
        | ENABLE_TRANSLATION_IT() << 4
        | ENABLE_TRANSLATION_PL() << 5
        | ENABLE_TRANSLATION_JA() << 6
        //
        ),
    .printer_code = PRINTER_CODE,
    .commit_dirty = FW_COMMIT_DIRTY,
    .has_bootloader = BOOTLOADER(),
};

void fill_project_version_no_dots(char *buffer, size_t buffer_size) {
    for (size_t version_i = 0, buffer_i = 0; version_i < strlen(project_version) && buffer_i < buffer_size - 1; ++version_i) {
        if (project_version[version_i] == '\0') {
            buffer[buffer_i] = '\0';
            break;
        }
        if (project_version[version_i] == '.') {
            continue;
        }
        buffer[buffer_i] = project_version[version_i];
        ++buffer_i;
    }
}

/* -===============================================(:>- */
// convert version from 1.2.34 to 1.2.3.4 (or 1.2.3 to 1.2.3.0)
void snake_version(char *version, int length) {
    strncpy(version, version::project_version_full, length - 2);
    if (version::project_version_full[length - 2] == '.') {
        version[length - 1] = '0';
        version[length - 0] = '.';
        version[length + 1] = version[length - 1];
        version[length + 2] = 0;
    } else {
        version[length - 1] = '.';
        version[length - 0] = version[length - 1];
        version[length + 1] = 0;
    }
}
/* -===============================================(:>- */

} // namespace version
