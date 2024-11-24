#include "driver_interface.h"
#include <stdexcept>

DriverInterface::DriverInterface() : m_driverHandle(INVALID_HANDLE_VALUE), m_connected(false) {
}

DriverInterface::~DriverInterface() {
    Disconnect();
}

bool DriverInterface::Initialize(const std::wstring& devicePath) {
    if (m_connected) {
        return true;
    }

    m_driverHandle = CreateFileW(
        devicePath.c_str(),
        GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        nullptr,
        OPEN_EXISTING,
        0,
        nullptr
    );

    if (m_driverHandle == INVALID_HANDLE_VALUE) {
        return false;
    }

    m_connected = true;
    return true;
}

void DriverInterface::Disconnect() {
    if (m_connected && m_driverHandle != INVALID_HANDLE_VALUE) {
        CloseHandle(m_driverHandle);
        m_driverHandle = INVALID_HANDLE_VALUE;
        m_connected = false;
    }
}

bool DriverInterface::SendIOCTL(DWORD ioctl, LPVOID inBuffer, DWORD inSize, LPVOID outBuffer, DWORD outSize) {
    if (!m_connected || m_driverHandle == INVALID_HANDLE_VALUE) {
        return false;
    }

    DWORD bytesReturned = 0;
    return DeviceIoControl(
        m_driverHandle,
        ioctl,
        inBuffer,
        inSize,
        outBuffer,
        outSize,
        &bytesReturned,
        nullptr
    );
}

bool DriverInterface::ProcessMessages() {
    // Handle any pending messages or operations
    return m_connected;
} 