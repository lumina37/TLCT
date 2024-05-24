#pragma once

#include "cmake.h"

#ifdef TLCT_HEADER_ONLY
#    define TLCT_API static
#else
#    ifdef TLCT_BUILD_SHARED_LIBS
#        ifdef _MSC_VER
#            ifdef _TLCT_BUILD_LIBS
#                define TLCT_API __declspec(dllexport)
#            else
#                define TLCT_API __declspec(dllimport)
#            endif
#        else
#            ifdef _TLCT_BUILD_LIBS
#                define TLCT_API __attribute__((visibility("default")))
#            else
#                define TLCT_API
#            endif
#        endif
#    endif
#endif
