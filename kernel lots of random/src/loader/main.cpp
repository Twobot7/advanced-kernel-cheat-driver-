#include "loader.h"
#include "mapper.h"
#include "protection.h"
#include <iostream>
#include <Windows.h>

int main(int argc, char* argv[]) {
    try {
        // Initialize protection and anti-debug
        if (!Protection::Initialize()) {
            throw std::runtime_error("Failed to initialize protection");
        }

        // Create loader instance
        auto loader = std::make_unique<Loader>();
        
        // Load and map driver
        if (!loader->LoadDriver(L"driver.sys")) {
            throw std::runtime_error("Failed to load driver");
        }

        std::cout << "Driver loaded successfully" << std::endl;
        
        // Initialize communication
        if (!loader->InitializeCommunication()) {
            throw std::runtime_error("Failed to initialize communication");
        }

        // Main loop
        while (true) {
            loader->Update();
            Sleep(100);
        }

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
} 