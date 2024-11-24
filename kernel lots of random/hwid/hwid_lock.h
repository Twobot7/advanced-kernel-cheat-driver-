#ifndef HWID_LOCK_H
#define HWID_LOCK_H

#include <ntddk.h>

NTSTATUS CollectHardwareIdentifiers(WCHAR* hwidBuffer, ULONG bufferSize);
NTSTATUS ValidateHardwareID(void);

#endif // HWID_LOCK_H
