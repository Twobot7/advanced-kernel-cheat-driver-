#pragma once
#include <ntddk.h>
#include <ntdef.h>

// Make sure NTSTATUS is defined
#ifndef _NTSTATUS_DEFINED
#define _NTSTATUS_DEFINED
typedef LONG NTSTATUS;
#endif

// Add common status codes if needed
#ifndef STATUS_SUCCESS
#define STATUS_SUCCESS                   ((NTSTATUS)0x00000000L)
#endif

#ifndef STATUS_INSUFFICIENT_RESOURCES  
#define STATUS_INSUFFICIENT_RESOURCES    ((NTSTATUS)0xC000009AL)
#endif

// Advanced kernel structure definitions
typedef struct _SYSTEM_MODULE_ENTRY {
    HANDLE Section;
    PVOID MappedBase;
    PVOID ImageBase;
    ULONG ImageSize;
    ULONG Flags;
    USHORT LoadOrderIndex;
    USHORT InitOrderIndex;
    USHORT LoadCount;
    USHORT OffsetToFileName;
    UCHAR FullPathName[256];
} SYSTEM_MODULE_ENTRY, *PSYSTEM_MODULE_ENTRY;

typedef struct _STEALTH_CONTEXT {
    PVOID OriginalBase;
    SIZE_T ImageSize;
    PVOID RelocatedBase;
    PVOID EncryptionKey;
    ULONG_PTR ObCallbackHandle;
    LIST_ENTRY DecoyList;
    KSPIN_LOCK ProtectionLock;
    PVOID HookedFunctions[32];
} STEALTH_CONTEXT, *PSTEALTH_CONTEXT; 

#define STEALTH_INIT_DRIVER_HIDE    0x00000010
#define STEALTH_INIT_CALLBACK_HIDE  0x00000020 