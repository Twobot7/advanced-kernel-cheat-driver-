#pragma once
#include <ntifs.h>

NTSTATUS CreateHiddenDevice(PDRIVER_OBJECT DriverObject, PDEVICE_OBJECT* DeviceObject);
NTSTATUS GenerateRandomDeviceName(PUNICODE_STRING DeviceName);
void SecureDeviceObject(PDEVICE_OBJECT DeviceObject); 