#include "utils.h"

ULONG ReadPCIConfig(ULONG Bus, ULONG Device, ULONG Function, ULONG Register) {
    ULONG data = 0;
    NTSTATUS status;

    status = HalGetBusDataByOffset(
        PCIConfiguration,
        Bus,
        (Device << 16) | Function,
        &data,
        Register,
        sizeof(ULONG)
    );

    if (status != sizeof(ULONG)) {
        LogError("Failed to read PCI config: Bus=%u, Dev=%u, Func=%u, Reg=%u", 
                Bus, Device, Function, Register);
        return 0;
    }

    return data;
}

VOID WritePCIConfig(ULONG Bus, ULONG Device, ULONG Function, ULONG Register, ULONG Value) {
    NTSTATUS status;

    status = HalSetBusDataByOffset(
        PCIConfiguration,
        Bus,
        (Device << 16) | Function,
        &Value,
        Register,
        sizeof(ULONG)
    );

    if (status != sizeof(ULONG)) {
        LogError("Failed to write PCI config: Bus=%u, Dev=%u, Func=%u, Reg=%u", 
                Bus, Device, Function, Register);
    }
}

PVOID AllocateNonPagedMemory(SIZE_T Size) {
    if (Size == 0) return NULL;

    PVOID memory = ExAllocatePool2(POOL_FLAG_NON_PAGED, Size, OVERLAY_TAG);
    if (memory) {
        RtlZeroMemory(memory, Size);
    }
    return memory;
}

VOID FreeNonPagedMemory(PVOID Memory) {
    if (Memory) {
        ExFreePoolWithTag(Memory, OVERLAY_TAG);
    }
}

PVOID MapPhysicalMemory(PHYSICAL_ADDRESS PhysicalAddress, SIZE_T Size) {
    if (Size == 0) return NULL;
    return MmMapIoSpace(PhysicalAddress, Size, MmNonCached);
}

VOID UnmapPhysicalMemory(PVOID VirtualAddress, SIZE_T Size) {
    if (VirtualAddress && Size > 0) {
        MmUnmapIoSpace(VirtualAddress, Size);
    }
}

VOID LogDebug(PCSTR Format, ...) {
    va_list args;
    va_start(args, Format);
    vDbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, Format, args);
    va_end(args);
}

VOID LogError(PCSTR Format, ...) {
    va_list args;
    va_start(args, Format);
    vDbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, Format, args);
    va_end(args);
}

VOID AcquireSpinLock(PKSPIN_LOCK SpinLock, PKIRQL OldIrql) {
    KeAcquireSpinLock(SpinLock, OldIrql);
}

VOID ReleaseSpinLock(PKSPIN_LOCK SpinLock, KIRQL OldIrql) {
    KeReleaseSpinLock(SpinLock, OldIrql);
}

VOID SecureZeroMemory(PVOID Ptr, SIZE_T Size) {
    if (Ptr && Size > 0) {
        RtlSecureZeroMemory(Ptr, Size);
    }
}

NTSTATUS SafeCopyMemory(PVOID Destination, PVOID Source, SIZE_T Size) {
    if (!Destination || !Source || Size == 0) {
        return STATUS_INVALID_PARAMETER;
    }

    __try {
        RtlCopyMemory(Destination, Source, Size);
        return STATUS_SUCCESS;
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {
        return STATUS_ACCESS_VIOLATION;
    }
}

BOOLEAN IsAddressValid(PVOID Address) {
    if (!Address) return FALSE;
    return MmIsAddressValid(Address);
}

BOOLEAN IsGpuPointerValid(PGPU_INFO GpuInfo) {
    if (!GpuInfo) return FALSE;
    if (!GpuInfo->FrameBufferVA) return FALSE;
    if (!GpuInfo->RegistersVA) return FALSE;
    if (GpuInfo->Vendor >= GPU_VENDOR_MAX) return FALSE;
    return TRUE;
}
