#pragma once
#include <Windows.h>

LONG WINAPI CustomExceptionHandler(EXCEPTION_POINTERS* ExceptionInfo);
bool IsBlacklistedProcess(const wchar_t* processName); 