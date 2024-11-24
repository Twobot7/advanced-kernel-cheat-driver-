#pragma once
#include "stealth_types.h"

// Decoy configuration
#define MAX_DECOY_REGIONS 16
#define MIN_DECOY_SIZE (PAGE_SIZE * 4)
#define MAX_DECOY_SIZE (PAGE_SIZE * 64)

typedef struct _DECOY_REGION {
    LIST_ENTRY ListEntry;
    PVOID BaseAddress;
    SIZE_T Size;
    ULONG LastModifiedTick;
    ULONG AccessCount;
    BOOLEAN IsActive;
} DECOY_REGION, *PDECOY_REGION;

NTSTATUS InitializeDecoys(IN PSTEALTH_CONTEXT Context);
VOID CleanupDecoys(IN PSTEALTH_CONTEXT Context);
VOID UpdateDecoyRegions(IN PSTEALTH_CONTEXT Context);
