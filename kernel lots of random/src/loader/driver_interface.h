#pragma once
#include <Windows.h>
#include <string>

class DriverInterface {
private:
    HANDLE m_driverHandle;
    bool m_connected;

    bool SendIOCTL(DWORD ioctl, LPVOID inBuffer, DWORD inSize, LPVOID outBuffer, DWORD outSize);

public:
    DriverInterface();
    ~DriverInterface();

    bool Initialize(const std::wstring& devicePath);
    void Disconnect();
    bool ProcessMessages();
    bool IsConnected() const { return m_connected; }
}; 