#pragma once

#ifdef _TLCT_LIB_HEADER_ONLY
#    define TLCT_API
#else
#    ifdef _TLCT_LIB_DYNAMIC
#        ifdef _MSC_VER
#            ifdef tlct_lib_dynamic_EXPORTS
#                define TLCT_API __declspec(dllexport)
#            else
#                define TLCT_API __declspec(dllimport)
#            endif
#        else
#            ifdef tlct_lib_dynamic_EXPORTS
#                define TLCT_API __attribute__((visibility("default")))
#            else
#                define TLCT_API
#            endif
#        endif
#    else
#        define TLCT_API
#    endif
#endif
