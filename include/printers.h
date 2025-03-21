//! @file
#pragma once

//! Printer variant
//!@{
#define PRINTER_IS_PRUSA_MK4()        (PRINTER_TYPE == 1 && PRINTER_VERSION == 4 && PRINTER_SUBVERSION == 0)
#define PRINTER_IS_PRUSA_MINI()       (PRINTER_TYPE == 2 && PRINTER_VERSION == 1 && PRINTER_SUBVERSION == 0)
#define PRINTER_IS_PRUSA_XL()         (PRINTER_TYPE == 3 && PRINTER_VERSION == 1 && PRINTER_SUBVERSION == 0)
#define PRINTER_IS_PRUSA_iX()         (PRINTER_TYPE == 4 && PRINTER_VERSION == 1 && PRINTER_SUBVERSION == 0)
#define PRINTER_IS_PRUSA_XL_DEV_KIT() (PRINTER_TYPE == 5 && PRINTER_VERSION == 1 && PRINTER_SUBVERSION == 0)
#define PRINTER_IS_PRUSA_MK3_5()      (PRINTER_TYPE == 1 && PRINTER_VERSION == 3 && PRINTER_SUBVERSION == 5)

#if PRINTER_IS_PRUSA_XL_DEV_KIT()
    // todo: for now, xl_dev_kit runs on XL
    #undef PRINTER_IS_PRUSA_XL
    #define PRINTER_IS_PRUSA_XL() true
#endif
//!@}
