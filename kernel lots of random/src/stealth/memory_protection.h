#pragma once
#include <ntddk.h>
#include "../common/types.h"

typedef struct _STEALTH_PROTECTION_STATE {
    KSPIN_LOCK StateLock;
    BOOLEAN IsInitialized;
    LIST_ENTRY ProtectedRegions;
} STEALTH_PROTECTION_STATE, *PSTEALTH_PROTECTION_STATE;

NTSTATUS InitializeMemoryProtection(VOID);
VOID CleanupMemoryProtection(VOID);
NTSTATUS ProtectMemoryRegion(PVOID BaseAddress, SIZE_T Size, BOOLEAN Encrypt); 