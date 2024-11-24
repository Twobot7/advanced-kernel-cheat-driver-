#include "amd.h"
#include "../common/utils.h"

static AMD_GPU_STATE g_AmdState = { 0 };

NTSTATUS AmdInitialize(PGPU_INFO GpuInfo) {
    NTSTATUS status = STATUS_SUCCESS;

    // Map AMD display registers
    g_AmdState.DisplayRegs = (PVOID)((ULONG_PTR)GpuInfo->RegistersVA + AMD_DISPLAY_BASE);

    // Get frame buffer location
    ULONG fbLocation = READ_REGISTER_ULONG(
        (PULONG)((ULONG_PTR)GpuInfo->RegistersVA + AMD_MC_FB_LOCATION)
    );

    // Calculate frame buffer address
    GpuInfo->FrameBufferPA.QuadPart = (ULONGLONG)(fbLocation & 0xFFFF0000) << 8;

    // Set initial mode
    status = AmdSetMode(GpuInfo, GpuInfo->ScreenWidth, GpuInfo->ScreenHeight, GpuInfo->Bpp);
    if (!NT_SUCCESS(status)) {
        LogError("AMD: Failed to set initial mode: 0x%08X", status);
        return status;
    }

    return STATUS_SUCCESS;
}

VOID AmdCleanup(PGPU_INFO GpuInfo) {
    RtlZeroMemory(&g_AmdState, sizeof(g_AmdState));
}

NTSTATUS AmdDrawPrimitive(PGPU_INFO GpuInfo, PVERTEX Vertices, ULONG Count) {
    if (!GpuInfo->FrameBufferVA || Count < 2) {
        return STATUS_INVALID_PARAMETER;
    }

    // Similar to NVIDIA implementation but with AMD-specific optimizations
    PULONG frameBuffer = (PULONG)GpuInfo->FrameBufferVA;
    
    for (ULONG i = 0; i < Count - 1; i += 2) {
        INT x1 = (INT)Vertices[i].x;
        INT y1 = (INT)Vertices[i].y;
        INT x2 = (INT)Vertices[i + 1].x;
        INT y2 = (INT)Vertices[i + 1].y;

        // Bresenham's line algorithm with AMD-specific memory access patterns
        INT dx = abs(x2 - x1);
        INT dy = abs(y2 - y1);
        INT sx = x1 < x2 ? 1 : -1;
        INT sy = y1 < y2 ? 1 : -1;
        INT err = dx - dy;

        while (1) {
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

    return STATUS_SUCCESS;
}

NTSTATUS AmdSetMode(PGPU_INFO GpuInfo, ULONG Width, ULONG Height, ULONG Bpp) {
    g_AmdState.CurrentWidth = Width;
    g_AmdState.CurrentHeight = Height;
    g_AmdState.CurrentBpp = Bpp;

    // AMD-specific mode setting registers
    WRITE_REGISTER_ULONG((PULONG)((ULONG_PTR)g_AmdState.DisplayRegs + 0x0), Width);
    WRITE_REGISTER_ULONG((PULONG)((ULONG_PTR)g_AmdState.DisplayRegs + 0x4), Height);
    WRITE_REGISTER_ULONG((PULONG)((ULONG_PTR)g_AmdState.DisplayRegs + 0x8), Bpp);

    return STATUS_SUCCESS;
}
