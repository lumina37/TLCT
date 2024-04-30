#include "cmake.h"

#if defined(_MSC_VER) && defined(TLCT_BUILD_SHARED_LIBS)
#    ifdef _TLCT_BUILD_LIBS
#        define TLCT_API __declspec(dllexport)
#    else
#        define TLCT_API __declspec(dllimport)
#    endif
#else
#    ifdef _TLCT_BUILD_LIBS
#        define TLCT_API __attribute__((visibility("default")))
#    else
#        define TLCT_API
#    endif
#endif
