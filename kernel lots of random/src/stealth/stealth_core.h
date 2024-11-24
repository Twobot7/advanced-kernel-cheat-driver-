#pragma once
#include "stealth_types.h"

// Advanced initialization flags
#define STEALTH_INIT_RELOCATE      0x00000001
#define STEALTH_INIT_ENCRYPT       0x00000002
#define STEALTH_INIT_HOOK          0x00000004
#define STEALTH_INIT_DECOY         0x00000008
#define STEALTH_INIT_FULL          0x0000000F

NTSTATUS InitializeStealthSystem(
    IN PDRIVER_OBJECT DriverObject,
    IN ULONG Flags,
    OUT PSTEALTH_CONTEXT* Context
);

void CleanupStealthSystem(
    IN PSTEALTH_CONTEXT Context,
    IN BOOLEAN SecureCleanup
); 