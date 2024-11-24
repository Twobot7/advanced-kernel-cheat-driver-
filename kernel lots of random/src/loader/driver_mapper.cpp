#include "driver_mapper.h"
#include <Windows.h>
#include <iostream>
#include <filesystem>

DriverMapper::DriverMapper() : m_serviceHandle(nullptr), m_loaded(false) {
}

DriverMapper::~DriverMapper() {
    UnloadDriver();
}

bool DriverMapper::LoadDriver(const std::wstring& driverPath, ULONG stealthFlags) {
    if (m_loaded) {
        return true;
    }

    // Verify driver file exists
    if (!std::filesystem::exists(driverPath)) {
        return false;
    }

    m_driverPath = driverPath;

    // Create and start the service
    if (!CreateDriverService() || !StartDriverService()) {
        DeleteDriverService();
        return false;
    }

    m_loaded = true;
    return true;
}

bool DriverMapper::CreateDriverService() {
    SC_HANDLE scm = OpenSCManager(nullptr, nullptr, SC_MANAGER_ALL_ACCESS);
    if (!scm) {
        return false;
    }

    m_serviceHandle = CreateServiceW(
        scm,
        L"MemoryDriver",  // Reference to DRIVER_NAME from driver_config.h
        L"MemoryDriver",
        SERVICE_ALL_ACCESS,
        SERVICE_KERNEL_DRIVER,
        SERVICE_DEMAND_START,
        SERVICE_ERROR_NORMAL,
        m_driverPath.c_str(),
        nullptr,
        nullptr,
        nullptr,
        nullptr,
        nullptr
    );

    CloseServiceHandle(scm);
    return m_serviceHandle != nullptr;
}

bool DriverMapper::StartDriverService() {
    if (!m_serviceHandle) {
        return false;
    }

    return StartServiceW(m_serviceHandle, 0, nullptr);
}

void DriverMapper::StopDriverService() {
    if (m_serviceHandle) {
        SERVICE_STATUS status;
        ControlService(m_serviceHandle, SERVICE_CONTROL_STOP, &status);
    }
}

void DriverMapper::DeleteDriverService() {
    if (m_serviceHandle) {
        DeleteService(m_serviceHandle);
        CloseServiceHandle(m_serviceHandle);
        m_serviceHandle = nullptr;
    }
}

bool DriverMapper::UnloadDriver() {
    if (!m_loaded) {
        return true;
    }

    StopDriverService();
    DeleteDriverService();
    m_loaded = false;
    return true;
} 