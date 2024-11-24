#pragma once
#include <Windows.h>

// This class handles communication with a signed vulnerable driver
// for kernel memory operations
class VulnerableDriver {
private:
    static HANDLE driverHandle;
    static bool LoadVulnerableDriver();

public:
    static bool Initialize();
    static bool Cleanup();
    static bool ReadKernelMemory(PVOID address, PVOID buffer, SIZE_T size);
    static bool WriteKernelMemory(PVOID address, PVOID buffer, SIZE_T size);
    static PVOID AllocateKernelPool(SIZE_T size);
    static bool FreeKernelPool(PVOID address);
}; 