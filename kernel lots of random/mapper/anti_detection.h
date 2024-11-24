#pragma once
#include <Windows.h>

class AntiDetection {
public:
    static bool PrepareEnvironment();
    static bool HideFromPiDDB();
    static bool PatchKernelCallbacks();
    static bool DisableDriverVerification();
    static bool HideMemoryAllocations();
    static bool RemoveDriverTraces();
}; 