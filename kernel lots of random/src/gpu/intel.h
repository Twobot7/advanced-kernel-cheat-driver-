#pragma once
#include "gpu_common.h"

// Intel-specific registers
#define INTEL_DISPLAY_PIPE_A           0x70000
#define INTEL_DISPLAY_PIPE_B           0x71000
#define INTEL_GTT_BASE                 0x800000

// Intel structures
typedef struct _INTEL_GPU_STATE {
    PVOID DisplayPipeA;
    PVOID DisplayPipeB;
    PVOID GTTBase;
    ULONG CurrentWidth;
    ULONG CurrentHeight;
    ULONG CurrentBpp;
    BOOLEAN IsPrimaryPipe;
} INTEL_GPU_STATE, *PINTEL_GPU_STATE;

// Function declarations
NTSTATUS IntelInitialize(PGPU_INFO GpuInfo);
VOID IntelCleanup(PGPU_INFO GpuInfo);
NTSTATUS IntelDrawPrimitive(PGPU_INFO GpuInfo, PVERTEX Vertices, ULONG Count);
NTSTATUS IntelSetMode(PGPU_INFO GpuInfo, ULONG Width, ULONG Height, ULONG Bpp);
