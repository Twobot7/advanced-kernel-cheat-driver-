#include "loader.h"
#include <iostream>
#include <stdexcept>

Loader::Loader() : m_initialized(false) {
    m_mapper = std::make_unique<DriverMapper>();
    m_interface = std::make_unique<DriverInterface>();
}

Loader::~Loader() {
    Cleanup();
}

bool Loader::LoadDriver(const std::wstring& driverPath) {
    try {
        // First map the driver
        if (!MapDriver(driverPath)) {
            std::cerr << "Failed to map driver" << std::endl;
            return false;
        }

        // Then setup communication
        if (!SetupCommunication()) {
            std::cerr << "Failed to setup communication" << std::endl;
            return false;
        }

        m_initialized = true;
        return true;
    }
    catch (const std::exception& e) {
        std::cerr << "Exception during driver loading: " << e.what() << std::endl;
        return false;
    }
}

bool Loader::MapDriver(const std::wstring& driverPath) {
    // Reference to stealth initialization flags from stealth_core.h
    const ULONG stealthFlags = STEALTH_INIT_FULL;  // Using all stealth features

    try {
        if (!m_mapper->LoadDriver(driverPath, stealthFlags)) {
            return false;
        }

        // Wait for driver to initialize
        Sleep(1000);
        return true;
    }
    catch (...) {
        return false;
    }
}

bool Loader::SetupCommunication() {
    try {
        // Reference to driver configuration from driver_config.h
        return m_interface->Initialize(DRIVER_DEVICE_PATH);
    }
    catch (...) {
        return false;
    }
}

bool Loader::InitializeCommunication() {
    if (!m_initialized) {
        std::cerr << "Driver not loaded" << std::endl;
        return false;
    }

    try {
        // Additional communication setup if needed
        return true;
    }
    catch (...) {
        return false;
    }
}

void Loader::Update() {
    if (!m_initialized) {
        return;
    }

    try {
        // Handle any periodic updates or monitoring
        m_interface->ProcessMessages();
    }
    catch (const std::exception& e) {
        std::cerr << "Error during update: " << e.what() << std::endl;
    }
}

void Loader::Cleanup() {
    if (m_initialized) {
        try {
            m_interface->Disconnect();
            m_mapper->UnloadDriver();
            m_initialized = false;
        }
        catch (const std::exception& e) {
            std::cerr << "Error during cleanup: " << e.what() << std::endl;
        }
    }
} 