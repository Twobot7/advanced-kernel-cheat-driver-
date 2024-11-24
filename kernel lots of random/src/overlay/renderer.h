#pragma once
#include "../common/types.h"

// Renderer configuration
typedef struct _RENDERER_CONFIG {
    BOOLEAN EnableVSync;
    BOOLEAN DoubleBuffering;
    ULONG MaxVertices;
    ULONG MaxPrimitives;
} RENDERER_CONFIG, *PRENDERER_CONFIG;

// Renderer state
typedef struct _RENDERER_STATE {
    BOOLEAN Initialized;
    POVERLAY_BUFFER CurrentBuffer;
    POVERLAY_BUFFER BackBuffer;
    LIST_ENTRY BufferList;
    KSPIN_LOCK BufferLock;
    RENDERER_CONFIG Config;
    PGPU_INFO GpuInfo;
} RENDERER_STATE, *PRENDERER_STATE;

// Function declarations
NTSTATUS InitializeRenderer(PGPU_INFO GpuInfo, PRENDERER_CONFIG Config);
VOID CleanupRenderer(VOID);
NTSTATUS BeginFrame(VOID);
NTSTATUS EndFrame(VOID);
NTSTATUS DrawVertices(PVERTEX Vertices, ULONG Count);
NTSTATUS SwapBuffers(VOID);
