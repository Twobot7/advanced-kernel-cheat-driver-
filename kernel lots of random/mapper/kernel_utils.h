#pragma once
#include <Windows.h>

class KernelAllocator {
public:
    static PVOID AllocateKernelMemory(SIZE_T size);
    static bool FreeKernelMemory(PVOID address);
    static bool WriteKernelMemory(PVOID destination, PVOID source, SIZE_T size);
    static bool ProtectKernelMemory(PVOID address, SIZE_T size, ULONG protection);
}; 