#pragma once
#include "../common/types.h"

typedef struct _BUFFER_MANAGER {
    LIST_ENTRY FreeBuffers;
    LIST_ENTRY UsedBuffers;
    KSPIN_LOCK BufferLock;
    ULONG TotalBuffers;
    ULONG UsedBufferCount;
    SIZE_T BufferSize;
} BUFFER_MANAGER, *PBUFFER_MANAGER;

NTSTATUS InitializeBufferManager(PBUFFER_MANAGER Manager, ULONG BufferCount, SIZE_T BufferSize);
VOID CleanupBufferManager(PBUFFER_MANAGER Manager);
NTSTATUS AllocateBuffer(PBUFFER_MANAGER Manager, POVERLAY_BUFFER* Buffer);
VOID ReleaseBuffer(PBUFFER_MANAGER Manager, POVERLAY_BUFFER Buffer);
NTSTATUS ResizeBufferPool(PBUFFER_MANAGER Manager, ULONG NewBufferCount);
