#pragma once
#include <Windows.h>

// Driver configuration
#define DRIVER_NAME L"MemoryDriver"
#define DRIVER_DEVICE_PATH L"\\\\.\\MemoryDriver"

// Stealth flags
#define STEALTH_INIT_BASIC    0x00000001
#define STEALTH_INIT_ADVANCED 0x00000002
#define STEALTH_INIT_FULL     (STEALTH_INIT_BASIC | STEALTH_INIT_ADVANCED) 