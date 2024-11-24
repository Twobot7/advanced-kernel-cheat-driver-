#pragma once
#include "gpu_common.h"

// AMD-specific registers
#define AMD_MC_FB_LOCATION              0x2180
#define AMD_CONFIG_MEMSIZE              0x5428
#define AMD_DISPLAY_BASE                0x6000

// AMD structures
typedef struct _AMD_GPU_STATE {
    PVOID DisplayRegs;
    ULONG CurrentWidth;
    ULONG CurrentHeight;
    ULONG CurrentBpp;
} AMD_GPU_STATE, *PAMD_GPU_STATE;

// Function declarations
NTSTATUS AmdInitialize(PGPU_INFO GpuInfo);
VOID AmdCleanup(PGPU_INFO GpuInfo);
NTSTATUS AmdDrawPrimitive(PGPU_INFO GpuInfo, PVERTEX Vertices, ULONG Count);
NTSTATUS AmdSetMode(PGPU_INFO GpuInfo, ULONG Width, ULONG Height, ULONG Bpp);
