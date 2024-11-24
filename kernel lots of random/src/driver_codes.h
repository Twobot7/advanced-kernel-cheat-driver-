#pragma once
// #include <ntifs.h>
#include "driver_codes.h"

// Request for read/writing memory from/to a process.
#define IOCTL_DRIVER_COPY_MEMORY ((ULONG)CTL_CODE(DRIVER_DEVICE_TYPE, 0x808, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS))
#define IOCTL_DRIVER_MOUSE_MOVE    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x801, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_DRIVER_MOUSE_CLICK   CTL_CODE(FILE_DEVICE_UNKNOWN, 0x802, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_DRIVER_KEY_PRESS     CTL_CODE(FILE_DEVICE_UNKNOWN, 0x803, METHOD_BUFFERED, FILE_ANY_ACCESS)

// Copy requeest data.
typedef struct _DRIVER_COPY_MEMORY {
	ULONGLONG Source; // Source buffer address.
	ULONGLONG Target; // Target buffer address.
	ULONGLONG Size; // Buffer size.
	ULONG ProcessId; // Target process ID.
	BOOLEAN Write; // TRUE if writing, FALSE if reading.
} DRIVER_COPY_MEMORY, *PDRIVER_COPY_MEMORY;
