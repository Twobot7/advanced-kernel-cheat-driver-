#pragma once
#include <Windows.h>

class Protection {
public:
    static bool Initialize();
    static void Update();
    static void Cleanup();

private:
    static bool InitializeAntiDebug();
    static bool InitializeIntegrityChecks();
    static bool SetupExceptionHandlers();
    static void MonitorSystemState();
}; 