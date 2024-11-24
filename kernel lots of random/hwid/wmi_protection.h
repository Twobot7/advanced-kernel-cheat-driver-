#ifndef WMI_PROTECTION_H
#define WMI_PROTECTION_H

#include <ntddk.h>
#include <wmilib.h>

// Function declarations
NTSTATUS InitializeWmiProtection(void);
void CleanupWmiProtection(void);
NTSTATUS RegisterWmiGuids(PDRIVER_OBJECT DriverObject);

#endif // WMI_PROTECTION_H 