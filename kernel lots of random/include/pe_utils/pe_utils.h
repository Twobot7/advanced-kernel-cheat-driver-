#pragma once
#include <Windows.h>

PIMAGE_NT_HEADERS GetNTHeaders(PVOID imageBase);
bool VerifyPEHeaders(PVOID imageBase); 