#pragma once
#include <ntddk.h>
#include "types.h"

// PCI configuration space access
ULONG ReadPCIConfig(ULONG Bus, ULONG Device, ULONG Function, ULONG Register);
VOID WritePCIConfig(ULONG Bus, ULONG Device, ULONG Function, ULONG Register, ULONG Value);

// Memory management
PVOID AllocateNonPagedMemory(SIZE_T Size);
VOID FreeNonPagedMemory(PVOID Memory);

// Memory mapping
PVOID MapPhysicalMemory(PHYSICAL_ADDRESS PhysicalAddress, SIZE_T Size);
VOID UnmapPhysicalMemory(PVOID VirtualAddress, SIZE_T Size);

// Debug logging
VOID LogDebug(PCSTR Format, ...);
VOID LogError(PCSTR Format, ...);

// Synchronization helpers
VOID AcquireSpinLock(PKSPIN_LOCK SpinLock, PKIRQL OldIrql);
VOID ReleaseSpinLock(PKSPIN_LOCK SpinLock, KIRQL OldIrql);

// Memory operations
VOID SecureZeroMemory(PVOID Ptr, SIZE_T Size);
NTSTATUS SafeCopyMemory(PVOID Destination, PVOID Source, SIZE_T Size);

// Validation
BOOLEAN IsAddressValid(PVOID Address);
BOOLEAN IsGpuPointerValid(PGPU_INFO GpuInfo);

// GPU detection
GPU_TYPE DetectGpuType(void);

// Add this definition
typedef enum _GPU_TYPE {
    GPU_TYPE_UNKNOWN = 0,
    GPU_TYPE_NVIDIA,
    GPU_TYPE_AMD,
    GPU_TYPE_INTEL
} GPU_TYPE;
