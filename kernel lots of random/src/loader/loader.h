#pragma once
#include <Windows.h>
#include <memory>
#include "driver_interface.h"
#include "mapper.h"

class Loader {
private:
    std::unique_ptr<DriverMapper> m_mapper;
    std::unique_ptr<DriverInterface> m_interface;
    bool m_initialized;
    
    bool MapDriver(const std::wstring& driverPath);
    bool SetupCommunication();
    void Cleanup();

public:
    Loader();
    ~Loader();

    bool LoadDriver(const std::wstring& driverPath);
    bool InitializeCommunication();
    void Update();
}; 