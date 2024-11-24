#include "gpu_common.h"
#include "nvidia.h"
#include "amd.h"
#include "intel.h"

// GPU vendor-specific function tables
PFN_GPU_INIT GpuInitFunctions[] = {
    NULL,               // UNKNOWN
    NvInitialize,       // NVIDIA
    AmdInitialize,      // AMD
    IntelInitialize     // Intel
};

PFN_GPU_CLEANUP GpuCleanupFunctions[] = {
    NULL,               // UNKNOWN
    NvCleanup,         // NVIDIA
    AmdCleanup,        // AMD
    IntelCleanup       // Intel
};

PFN_GPU_DRAW_PRIMITIVE GpuDrawFunctions[] = {
    NULL,               // UNKNOWN
    NvDrawPrimitive,    // NVIDIA
    AmdDrawPrimitive,   // AMD
    IntelDrawPrimitive  // Intel
};

NTSTATUS DetectPrimaryGPU(PGPU_INFO GpuInfo) {
    NTSTATUS status = STATUS_GPU_NOT_FOUND;
    
    for (ULONG bus = 0; bus < 256; bus++) {
        for (ULONG dev = 0; dev < 32; dev++) {
            for (ULONG func = 0; func < 8; func++) {
                // Read class code and vendor ID
                ULONG classCode = ReadPCIConfig(bus, dev, func, PCI_CONFIG_REVISION) >> 24;
                ULONG vendorId = ReadPCIConfig(bus, dev, func, PCI_CONFIG_VENDOR_ID) & 0xFFFF;
                
                if (classCode != PCI_CLASS_DISPLAY) {
                    continue;
                }

                GpuInfo->VendorId = vendorId;
                GpuInfo->DeviceId = ReadPCIConfig(bus, dev, func, PCI_CONFIG_DEVICE_ID) >> 16;

                switch (vendorId) {
                    case PCI_VENDOR_NVIDIA:
                        GpuInfo->Vendor = GPU_VENDOR_NVIDIA;
                        status = STATUS_SUCCESS;
                        break;
                        
                    case PCI_VENDOR_AMD:
                        GpuInfo->Vendor = GPU_VENDOR_AMD;
                        status = STATUS_SUCCESS;
                        break;
                        
                    case PCI_VENDOR_INTEL:
                        GpuInfo->Vendor = GPU_VENDOR_INTEL;
                        status = STATUS_SUCCESS;
                        break;
                }

                if (NT_SUCCESS(status)) {
                    // Read BAR0 (memory-mapped registers)
                    ULONG bar0Low = ReadPCIConfig(bus, dev, func, PCI_CONFIG_BAR0);
                    ULONG bar0High = ReadPCIConfig(bus, dev, func, PCI_CONFIG_BAR1);
                    
                    GpuInfo->FrameBufferPA.LowPart = bar0Low & ~0xF;  // Clear flags
                    GpuInfo->FrameBufferPA.HighPart = bar0High;
                    
                    return status;
                }
            }
        }
    }
    
    return status;
}

NTSTATUS MapGPURegisters(PGPU_INFO GpuInfo) {
    if (!GpuInfo || GpuInfo->Vendor == GPU_VENDOR_UNKNOWN) {
        return STATUS_INVALID_PARAMETER;
    }

    // Map the GPU registers
    GpuInfo->RegistersVA = MapPhysicalMemory(
        GpuInfo->FrameBufferPA,
        PAGE_SIZE * 1024  // Map 4MB initially
    );

    if (!GpuInfo->RegistersVA) {
        LogError("Failed to map GPU registers");
        return STATUS_UNSUCCESSFUL;
    }

    return STATUS_SUCCESS;
}

VOID UnmapGPURegisters(PGPU_INFO GpuInfo) {
    if (GpuInfo && GpuInfo->RegistersVA) {
        UnmapPhysicalMemory(GpuInfo->RegistersVA, PAGE_SIZE * 1024);
        GpuInfo->RegistersVA = NULL;
    }
}

NTSTATUS InitializeGPU(PGPU_INFO GpuInfo) {
    if (!GpuInfo || GpuInfo->Vendor >= GPU_VENDOR_MAX) {
        return STATUS_INVALID_PARAMETER;
    }

    NTSTATUS status = MapGPURegisters(GpuInfo);
    if (!NT_SUCCESS(status)) {
        return status;
    }

    PFN_GPU_INIT initFunc = GpuInitFunctions[GpuInfo->Vendor];
    if (!initFunc) {
        UnmapGPURegisters(GpuInfo);
        return STATUS_NOT_SUPPORTED;
    }

    status = initFunc(GpuInfo);
    if (!NT_SUCCESS(status)) {
        UnmapGPURegisters(GpuInfo);
        return status;
    }

    return STATUS_SUCCESS;
}

VOID CleanupGPU(PGPU_INFO GpuInfo) {
    if (!GpuInfo || GpuInfo->Vendor >= GPU_VENDOR_MAX) {
        return;
    }

    PFN_GPU_CLEANUP cleanupFunc = GpuCleanupFunctions[GpuInfo->Vendor];
    if (cleanupFunc) {
        cleanupFunc(GpuInfo);
    }

    UnmapGPURegisters(GpuInfo);
}
