#pragma once

#include "cmake.h"

#define TLCT_API

#if !defined(TLCT_HEADER_ONLY) && defined(TLCT_BUILD_SHARED_LIBS)
#    ifdef _MSC_VER
#        ifdef _TLCT_BUILD_LIBS
#            define TLCT_API __declspec(dllexport)
#        else
#            define TLCT_API __declspec(dllimport)
#        endif
#    else
#        ifdef _TLCT_BUILD_LIBS
#            define TLCT_API __attribute__((visibility("default")))
#        else
#            define TLCT_API
#        endif
#    endif
#endif
