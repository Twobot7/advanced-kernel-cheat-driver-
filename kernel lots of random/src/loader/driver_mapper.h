#pragma once
#include <Windows.h>
#include <string>

class DriverMapper {
private:
    HANDLE m_serviceHandle;
    std::wstring m_driverPath;
    bool m_loaded;

    bool CreateDriverService();
    bool StartDriverService();
    void StopDriverService();
    void DeleteDriverService();

public:
    DriverMapper();
    ~DriverMapper();

    bool LoadDriver(const std::wstring& driverPath, ULONG stealthFlags);
    bool UnloadDriver();
    bool IsLoaded() const { return m_loaded; }
}; 