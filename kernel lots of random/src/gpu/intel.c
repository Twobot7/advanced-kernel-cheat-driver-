#include "intel.h"
#include "../common/utils.h"

static INTEL_GPU_STATE g_IntelState = { 0 };

NTSTATUS IntelInitialize(PGPU_INFO GpuInfo) {
    NTSTATUS status = STATUS_SUCCESS;

    // Map Intel display registers
    g_IntelState.DisplayPipeA = (PVOID)((ULONG_PTR)GpuInfo->RegistersVA + INTEL_DISPLAY_PIPE_A);
    g_IntelState.DisplayPipeB = (PVOID)((ULONG_PTR)GpuInfo->RegistersVA + INTEL_DISPLAY_PIPE_B);
    g_IntelState.GTTBase = (PVOID)((ULONG_PTR)GpuInfo->RegistersVA + INTEL_GTT_BASE);

    // Determine primary display pipe
    ULONG pipeAStatus = READ_REGISTER_ULONG((PULONG)g_IntelState.DisplayPipeA);
    ULONG pipeBStatus = READ_REGISTER_ULONG((PULONG)g_IntelState.DisplayPipeB);
    g_IntelState.IsPrimaryPipe = (pipeAStatus & 0x1) != 0;

    // Set initial mode
    status = IntelSetMode(GpuInfo, GpuInfo->ScreenWidth, GpuInfo->ScreenHeight, GpuInfo->Bpp);
    if (!NT_SUCCESS(status)) {
        LogError("Intel: Failed to set initial mode: 0x%08X", status);
        return status;
    }

    return STATUS_SUCCESS;
}

VOID IntelCleanup(PGPU_INFO GpuInfo) {
    RtlZeroMemory(&g_IntelState, sizeof(g_IntelState));
}

NTSTATUS IntelDrawPrimitive(PGPU_INFO GpuInfo, PVERTEX Vertices, ULONG Count) {
    if (!GpuInfo->FrameBufferVA || Count < 2) {
        return STATUS_INVALID_PARAMETER;
    }

    // Intel-specific implementation with GTT-aware memory access
    PULONG frameBuffer = (PULONG)GpuInfo->FrameBufferVA;

    for (ULONG i = 0; i < Count - 1; i += 2) {
        INT x1 = (INT)Vertices[i].x;
        INT y1 = (INT)Vertices[i].y;
        INT x2 = (INT)Vertices[i + 1].x;
        INT y2 = (INT)Vertices[i + 1].y;

        // Bresenham's algorithm with Intel-specific optimizations
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

NTSTATUS IntelSetMode(PGPU_INFO GpuInfo, ULONG Width, ULONG Height, ULONG Bpp) {
    PVOID activePipe = g_IntelState.IsPrimaryPipe ? 
                       g_IntelState.DisplayPipeA : 
                       g_IntelState.DisplayPipeB;

    g_IntelState.CurrentWidth = Width;
    g_IntelState.CurrentHeight = Height;
    g_IntelState.CurrentBpp = Bpp;

    // Intel-specific mode setting
    WRITE_REGISTER_ULONG((PULONG)((ULONG_PTR)activePipe + 0x0), Width);
    WRITE_REGISTER_ULONG((PULONG)((ULONG_PTR)activePipe + 0x4), Height);
    WRITE_REGISTER_ULONG((PULONG)((ULONG_PTR)activePipe + 0x8), Bpp);

    return STATUS_SUCCESS;
}
