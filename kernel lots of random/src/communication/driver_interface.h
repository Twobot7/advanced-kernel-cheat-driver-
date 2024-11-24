#pragma once
#include <Windows.h>

class DriverInterface {
private:
    static HANDLE driverHandle;
    static bool initialized;

public:
    static bool Initialize();
    static bool Cleanup();
    
    template<typename T>
    static bool SendCommand(DWORD ioctl, const T& input) {
        if (!initialized) return false;
        
        DWORD bytesReturned;
        return DeviceIoControl(
            driverHandle,
            ioctl,
            (LPVOID)&input,
            sizeof(T),
            nullptr,
            0,
            &bytesReturned,
            nullptr
        );
    }
}; 