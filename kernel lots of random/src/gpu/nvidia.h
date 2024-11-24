#pragma once
#include "gpu_common.h"

// NVIDIA-specific registers
#define NV_PMC_BOOT_0                  0x000000
#define NV_PMC_INTR_0                  0x000100
#define NV_PVIDEO_BASE                 0x008000
#define NV_PCRTC_BASE                  0x600000
#define NV_PRAMDAC_BASE               0x680000
#define NV_FIFO_BASE                  0x800000

// NVIDIA-specific structures
typedef struct _NV_GPU_STATE {
    PVOID PVIDEO;
    PVOID PCRTC;
    PVOID PRAMDAC;
    PVOID FIFO;
    ULONG CurrentWidth;
    ULONG CurrentHeight;
    ULONG CurrentBpp;
    BOOLEAN IsInitialized;
} NV_GPU_STATE, *PNV_GPU_STATE;

// Function declarations
NTSTATUS NvInitialize(PGPU_INFO GpuInfo);
VOID NvCleanup(PGPU_INFO GpuInfo);
NTSTATUS NvDrawPrimitive(PGPU_INFO GpuInfo, PVERTEX Vertices, ULONG Count);
NTSTATUS NvSetMode(PGPU_INFO GpuInfo, ULONG Width, ULONG Height, ULONG Bpp);
