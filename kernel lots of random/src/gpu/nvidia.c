#include "nvidia.h"

static NV_GPU_STATE g_NvidiaState = { 0 };

NTSTATUS NvInitialize(PGPU_INFO GpuInfo) {
    if (!GpuInfo || !GpuInfo->RegistersVA) {
        return STATUS_INVALID_PARAMETER;
    }

    // Allocate vendor-specific state
    PNV_GPU_STATE nvState = (PNV_GPU_STATE)AllocateNonPagedMemory(sizeof(NV_GPU_STATE));
    if (!nvState) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    // Map NVIDIA registers
    nvState->PVIDEO = (PVOID)((ULONG_PTR)GpuInfo->RegistersVA + NV_PVIDEO_BASE);
    nvState->PCRTC = (PVOID)((ULONG_PTR)GpuInfo->RegistersVA + NV_PCRTC_BASE);
    nvState->PRAMDAC = (PVOID)((ULONG_PTR)GpuInfo->RegistersVA + NV_PRAMDAC_BASE);
    nvState->FIFO = (PVOID)((ULONG_PTR)GpuInfo->RegistersVA + NV_FIFO_BASE);

    // Enable memory access
    WRITE_REGISTER_ULONG(
        (PULONG)((ULONG_PTR)GpuInfo->RegistersVA + NV_PMC_BOOT_0),
        0x00000001
    );

    // Store the state in GPU_INFO
    GpuInfo->VendorSpecific = nvState;
    nvState->IsInitialized = TRUE;

    return STATUS_SUCCESS;
}

VOID NvCleanup(PGPU_INFO GpuInfo) {
    if (GpuInfo && GpuInfo->VendorSpecific) {
        PNV_GPU_STATE nvState = (PNV_GPU_STATE)GpuInfo->VendorSpecific;
        
        // Disable memory access
        if (GpuInfo->RegistersVA) {
            WRITE_REGISTER_ULONG(
                (PULONG)((ULONG_PTR)GpuInfo->RegistersVA + NV_PMC_BOOT_0),
                0x00000000
            );
        }

        FreeNonPagedMemory(nvState);
        GpuInfo->VendorSpecific = NULL;
    }
}

NTSTATUS NvDrawPrimitive(PGPU_INFO GpuInfo, PVERTEX Vertices, ULONG Count) {
    if (!GpuInfo || !GpuInfo->VendorSpecific || !Vertices || Count < 2) {
        return STATUS_INVALID_PARAMETER;
    }

    PNV_GPU_STATE nvState = (PNV_GPU_STATE)GpuInfo->VendorSpecific;
    if (!nvState->IsInitialized) {
        return STATUS_DEVICE_NOT_READY;
    }

    // Acquire both GPU and memory protection locks
    KIRQL oldIrql, memIrql;
    KeAcquireSpinLock(&GpuInfo->Lock, &oldIrql);
    KeAcquireSpinLock(&g_ProtectionState.StateLock, &memIrql);

    // Direct frame buffer manipulation for line drawing
    PULONG frameBuffer = (PULONG)GpuInfo->FrameBufferVA;
    
    for (ULONG i = 0; i < Count - 1; i += 2) {
        // Convert floating-point coordinates to integers
        INT x1 = (INT)Vertices[i].x;
        INT y1 = (INT)Vertices[i].y;
        INT x2 = (INT)Vertices[i + 1].x;
        INT y2 = (INT)Vertices[i + 1].y;

        // Implement Bresenham's line algorithm
        INT dx = abs(x2 - x1);
        INT dy = abs(y2 - y1);
        INT sx = x1 < x2 ? 1 : -1;
        INT sy = y1 < y2 ? 1 : -1;
        INT err = dx - dy;

        while (TRUE) {
            if (x1 >= 0 && x1 < (INT)GpuInfo->ScreenWidth && 
                y1 >= 0 && y1 < (INT)GpuInfo->ScreenHeight) {
                
                ULONG color = (Vertices[i].color.a << 24) |
                             (Vertices[i].color.r << 16) |
                             (Vertices[i].color.g << 8) |
                             Vertices[i].color.b;
                
                frameBuffer[y1 * GpuInfo->ScreenWidth + x1] = color;
            }

            if (x1 == x2 && y1 == y2) break;
            
            INT e2 = 2 * err;
            if (e2 > -dy) { err -= dy; x1 += sx; }
            if (e2 < dx) { err += dx; y1 += sy; }
        }
    }

    KeReleaseSpinLock(&g_ProtectionState.StateLock, memIrql);
    KeReleaseSpinLock(&GpuInfo->Lock, oldIrql);
    return STATUS_SUCCESS;
}

NTSTATUS NvSetMode(PGPU_INFO GpuInfo, ULONG Width, ULONG Height, ULONG Bpp) {
    if (!GpuInfo || !GpuInfo->VendorSpecific) {
        return STATUS_INVALID_PARAMETER;
    }

    PNV_GPU_STATE nvState = (PNV_GPU_STATE)GpuInfo->VendorSpecific;
    
    KIRQL oldIrql;
    KeAcquireSpinLock(&GpuInfo->Lock, &oldIrql);

    nvState->CurrentWidth = Width;
    nvState->CurrentHeight = Height;
    nvState->CurrentBpp = Bpp;

    // Set CRTC timing registers
    WRITE_REGISTER_ULONG((PULONG)((ULONG_PTR)nvState->PCRTC + 0x00), Width);
    WRITE_REGISTER_ULONG((PULONG)((ULONG_PTR)nvState->PCRTC + 0x04), Height);
    WRITE_REGISTER_ULONG((PULONG)((ULONG_PTR)nvState->PCRTC + 0x08), Bpp);

    KeReleaseSpinLock(&GpuInfo->Lock, oldIrql);
    return STATUS_SUCCESS;
}
