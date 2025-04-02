#pragma once

#if defined(EXTENSION_VARIANT) && EXTENSION_VARIANT == EXTENSION_STANDARD
    #define EXTENSION_IS_STANDARD() 1
#elif defined(EXTENSION_VARIANT) && EXTENSION_VARIANT == EXTENSION_IX
    #define EXTENSION_IS_IX() 1
#else
    #error Please define the EXTENSION_VARIANT macro
#endif

#ifndef EXTENSION_IS_STANDARD
    #define EXTENSION_IS_STANDARD() 0
#endif

#ifndef EXTENSION_IS_IX
    #define EXTENSION_IS_IX() 0
#endif
