#include "anti_debug/anti_debug.h"

LONG WINAPI CustomExceptionHandler(EXCEPTION_POINTERS* ExceptionInfo) {
    return EXCEPTION_CONTINUE_SEARCH;
}

bool IsBlacklistedProcess(const wchar_t* processName) {
    // Add your blacklist logic here
    return false;
} 