#include "renderer.h"
#include "primitives.h"
#include "../common/utils.h"

static RENDERER_STATE g_RendererState = { 0 };

NTSTATUS InitializeRenderer(PGPU_INFO GpuInfo, PRENDERER_CONFIG Config) {
    if (!GpuInfo || !Config) {
        return STATUS_INVALID_PARAMETER;
    }

    // Initialize renderer state
    RtlZeroMemory(&g_RendererState, sizeof(RENDERER_STATE));
    g_RendererState.GpuInfo = GpuInfo;
    g_RendererState.Config = *Config;
    
    InitializeListHead(&g_RendererState.BufferList);
    KeInitializeSpinLock(&g_RendererState.BufferLock);

    // Allocate buffers
    for (ULONG i = 0; i < (Config->DoubleBuffering ? 2 : 1); i++) {
        POVERLAY_BUFFER buffer = AllocateNonPagedMemory(sizeof(OVERLAY_BUFFER));
        if (!buffer) {
            CleanupRenderer();
            return STATUS_INSUFFICIENT_RESOURCES;
        }

        // Allocate buffer memory
        buffer->Size = Config->MaxVertices * sizeof(VERTEX);
        buffer->VirtualAddress = AllocateNonPagedMemory(buffer->Size);
        if (!buffer->VirtualAddress) {
            FreeNonPagedMemory(buffer);
            CleanupRenderer();
            return STATUS_INSUFFICIENT_RESOURCES;
        }

        // Get physical address for GPU access
        buffer->PhysicalAddress = MmGetPhysicalAddress(buffer->VirtualAddress);
        buffer->InUse = FALSE;

        InsertTailList(&g_RendererState.BufferList, &buffer->ListEntry);
    }

    // Set initial buffer
    g_RendererState.CurrentBuffer = CONTAINING_RECORD(
        g_RendererState.BufferList.Flink,
        OVERLAY_BUFFER,
        ListEntry
    );

    if (Config->DoubleBuffering) {
        g_RendererState.BackBuffer = CONTAINING_RECORD(
            g_RendererState.BufferList.Blink,
            OVERLAY_BUFFER,
            ListEntry
        );
    }

    g_RendererState.Initialized = TRUE;
    return STATUS_SUCCESS;
}

VOID CleanupRenderer(VOID) {
    KIRQL oldIrql;
    KeAcquireSpinLock(&g_RendererState.BufferLock, &oldIrql);

    while (!IsListEmpty(&g_RendererState.BufferList)) {
        PLIST_ENTRY entry = RemoveHeadList(&g_RendererState.BufferList);
        POVERLAY_BUFFER buffer = CONTAINING_RECORD(entry, OVERLAY_BUFFER, ListEntry);
        
        if (buffer->VirtualAddress) {
            FreeNonPagedMemory(buffer->VirtualAddress);
        }
        FreeNonPagedMemory(buffer);
    }

    KeReleaseSpinLock(&g_RendererState.BufferLock, oldIrql);
    RtlZeroMemory(&g_RendererState, sizeof(RENDERER_STATE));
}

NTSTATUS BeginFrame(VOID) {
    if (!g_RendererState.Initialized) {
        return STATUS_DEVICE_NOT_READY;
    }

    KIRQL oldIrql;
    KeAcquireSpinLock(&g_RendererState.BufferLock, &oldIrql);

    if (g_RendererState.Config.DoubleBuffering) {
        g_RendererState.CurrentBuffer = g_RendererState.BackBuffer;
    }

    g_RendererState.CurrentBuffer->InUse = TRUE;
    
    // Clear buffer
    RtlZeroMemory(
        g_RendererState.CurrentBuffer->VirtualAddress,
        g_RendererState.CurrentBuffer->Size
    );

    KeReleaseSpinLock(&g_RendererState.BufferLock, oldIrql);
    return STATUS_SUCCESS;
}

NTSTATUS EndFrame(VOID) {
    if (!g_RendererState.Initialized) {
        return STATUS_DEVICE_NOT_READY;
    }

    KIRQL oldIrql;
    KeAcquireSpinLock(&g_RendererState.BufferLock, &oldIrql);

    // Submit buffer to GPU
    switch (g_RendererState.GpuInfo->Vendor) {
        case GPU_VENDOR_NVIDIA:
            NvDrawPrimitive(
                g_RendererState.GpuInfo,
                g_RendererState.CurrentBuffer->VirtualAddress,
                g_RendererState.Config.MaxVertices
            );
            break;

        case GPU_VENDOR_AMD:
            AmdDrawPrimitive(
                g_RendererState.GpuInfo,
                g_RendererState.CurrentBuffer->VirtualAddress,
                g_RendererState.Config.MaxVertices
            );
            break;

        case GPU_VENDOR_INTEL:
            IntelDrawPrimitive(
                g_RendererState.GpuInfo,
                g_RendererState.CurrentBuffer->VirtualAddress,
                g_RendererState.Config.MaxVertices
            );
            break;
    }

    g_RendererState.CurrentBuffer->InUse = FALSE;

    KeReleaseSpinLock(&g_RendererState.BufferLock, oldIrql);
    return STATUS_SUCCESS;
}
