#include "device_protection.h"

NTSTATUS CreateHiddenDevice(PDRIVER_OBJECT DriverObject, PDEVICE_OBJECT* DeviceObject) {
    UNICODE_STRING deviceName;
    NTSTATUS status = GenerateRandomDeviceName(&deviceName);
    if (!NT_SUCCESS(status)) {
        return status;
    }
    
    ULONG deviceCharacteristics = FILE_DEVICE_SECURE_OPEN | 
                                 FILE_DEVICE_CHARACTERISTICS_HIDDEN |
                                 FILE_DEVICE_EXCLUSIVE;
    
    status = IoCreateDevice(DriverObject, 
                           sizeof(PVOID),
                           &deviceName,
                           FILE_DEVICE_UNKNOWN,
                           deviceCharacteristics,
                           TRUE,
                           DeviceObject);
                           
    if (NT_SUCCESS(status)) {
        SecureDeviceObject(*DeviceObject);
    }
    
    return status;
}

