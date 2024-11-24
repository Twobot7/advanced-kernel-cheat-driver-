#include "protection.h"
#include "anti_debug.h"
#include "integrity.h"
#include <TlHelp32.h>

bool Protection::Initialize() {
    if (!InitializeAntiDebug()) return false;
    if (!InitializeIntegrityChecks()) return false;
    if (!SetupExceptionHandlers()) return false;
    
    return true;
}

bool Protection::InitializeAntiDebug() {
    // Implement various anti-debug checks
    if (IsDebuggerPresent()) return false;
    if (CheckRemoteDebuggerPresent(GetCurrentProcess(), nullptr)) return false;
    
    // Additional anti-debug measures
    SetUnhandledExceptionFilter(CustomExceptionHandler);
    
    return true;
}

void Protection::Update() {
    static DWORD lastCheck = 0;
    DWORD currentTick = GetTickCount();
    
    if (currentTick - lastCheck > 1000) {
        MonitorSystemState();
        lastCheck = currentTick;
    }
}

void Protection::MonitorSystemState() {
    // Check for suspicious processes
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot != INVALID_HANDLE_VALUE) {
        PROCESSENTRY32W processEntry = { sizeof(PROCESSENTRY32W) };
        
        if (Process32FirstW(snapshot, &processEntry)) {
            do {
                if (IsBlacklistedProcess(processEntry.szExeFile)) {
                    // Handle detection
                    Protection::Cleanup();
                    ExitProcess(0);
                }
            } while (Process32NextW(snapshot, &processEntry));
        }
        CloseHandle(snapshot);
    }
} 