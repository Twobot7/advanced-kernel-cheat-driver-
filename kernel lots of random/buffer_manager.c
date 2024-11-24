#include "buffer_manager.h"
#include "../common/utils.h"

NTSTATUS InitializeBufferManager(PBUFFER_MANAGER Manager, ULONG BufferCount, SIZE_T BufferSize) {
    if (!Manager || BufferCount == 0 || BufferSize == 0) {
        return STATUS_INVALID_PARAMETER;
    }

    RtlZeroMemory(Manager, sizeof(BUFFER_MANAGER));
    InitializeListHead(&Manager->FreeBuffers);
    InitializeListHead(&Manager->UsedBuffers);
    KeInitializeSpinLock(&Manager->BufferLock);
    
    Manager->BufferSize = BufferSize;
    Manager->TotalBuffers = 0;
    Manager->UsedBufferCount = 0;

    for (ULONG i = 0; i < BufferCount; i++) {
        POVERLAY_BUFFER buffer = AllocateNonPagedMemory(sizeof(OVERLAY_BUFFER));
        if (!buffer) {
            CleanupBufferManager(Manager);
            return STATUS_INSUFFICIENT_RESOURCES;
        }

        buffer->VirtualAddress = AllocateNonPagedMemory(BufferSize);
        if (!buffer->VirtualAddress) {
            FreeNonPagedMemory(buffer);
            CleanupBufferManager(Manager);
            return STATUS_INSUFFICIENT_RESOURCES;
        }

        buffer->PhysicalAddress = MmGetPhysicalAddress(buffer->VirtualAddress);
        buffer->Size = BufferSize;
        buffer->InUse = FALSE;

        InsertTailList(&Manager->FreeBuffers, &buffer->ListEntry);
        Manager->TotalBuffers++;
    }

    return STATUS_SUCCESS;
}

VOID CleanupBufferManager(PBUFFER_MANAGER Manager) {
    if (!Manager) {
        return;
    }

    KIRQL oldIrql;
    KeAcquireSpinLock(&Manager->BufferLock, &oldIrql);

    // Clean up free buffers
    while (!IsListEmpty(&Manager->FreeBuffers)) {
        PLIST_ENTRY entry = RemoveHeadList(&Manager->FreeBuffers);
        POVERLAY_BUFFER buffer = CONTAINING_RECORD(entry, OVERLAY_BUFFER, ListEntry);
        
        if (buffer->VirtualAddress) {
            FreeNonPagedMemory(buffer->VirtualAddress);
        }
        FreeNonPagedMemory(buffer);
    }

    // Clean up used buffers
    while (!IsListEmpty(&Manager->UsedBuffers)) {
        PLIST_ENTRY entry = RemoveHeadList(&Manager->UsedBuffers);
        POVERLAY_BUFFER buffer = CONTAINING_RECORD(entry, OVERLAY_BUFFER, ListEntry);
        
        if (buffer->VirtualAddress) {
            FreeNonPagedMemory(buffer->VirtualAddress);
        }
        FreeNonPagedMemory(buffer);
    }

    KeReleaseSpinLock(&Manager->BufferLock, oldIrql);
    RtlZeroMemory(Manager, sizeof(BUFFER_MANAGER));
}

NTSTATUS AllocateBuffer(PBUFFER_MANAGER Manager, POVERLAY_BUFFER* Buffer) {
    if (!Manager || !Buffer) {
        return STATUS_INVALID_PARAMETER;
    }

    KIRQL oldIrql;
    KeAcquireSpinLock(&Manager->BufferLock, &oldIrql);

    if (IsListEmpty(&Manager->FreeBuffers)) {
        KeReleaseSpinLock(&Manager->BufferLock, oldIrql);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    PLIST_ENTRY entry = RemoveHeadList(&Manager->FreeBuffers);
    POVERLAY_BUFFER buffer = CONTAINING_RECORD(entry, OVERLAY_BUFFER, ListEntry);
    
    buffer->InUse = TRUE;
    InsertTailList(&Manager->UsedBuffers, &buffer->ListEntry);
    Manager->UsedBufferCount++;

    *Buffer = buffer;

    KeReleaseSpinLock(&Manager->BufferLock, oldIrql);
    return STATUS_SUCCESS;
}

VOID ReleaseBuffer(PBUFFER_MANAGER Manager, POVERLAY_BUFFER Buffer) {
    if (!Manager || !Buffer) {
        return;
    }

    KIRQL oldIrql;
    KeAcquireSpinLock(&Manager->BufferLock, &oldIrql);

    if (Buffer->InUse) {
        RemoveEntryList(&Buffer->ListEntry);
        Buffer->InUse = FALSE;
        InsertTailList(&Manager->FreeBuffers, &Buffer->ListEntry);
        Manager->UsedBufferCount--;
    }

    KeReleaseSpinLock(&Manager->BufferLock, oldIrql);
}
