#include <ntifs.h>
#include <ntddk.h>
#include <wdm.h>
#include <ntimage.h>
#include <windef.h>
#include <ntddkbd.h>
#include <aux_klib.h>
#include "stealth/stealth_core.h"
#include "stealth/device_protection.h"
#include "driver_codes.h"
#include "driver_config.h"
#include "common/types.h"
#include "common/utils.h"
#include "gpu/gpu_common.h"
#include "overlay/primitives.h"
#include "overlay/renderer.h"
#include "input/input_control.h"
#include "overlay/renderer.h"
#include "input/input_control.h"

// Copies virtual memory from one process to another.
NTKERNELAPI NTSTATUS NTAPI MmCopyVirtualMemory(
	IN PEPROCESS FromProcess,
	IN PVOID FromAddress,
	IN PEPROCESS ToProcess,
	OUT PVOID ToAddress,
	IN SIZE_T BufferSize,
	IN KPROCESSOR_MODE PreviousMode,
	OUT PSIZE_T NumberOfBytesCopied
);

// Forward declaration for suppressing code analysis warnings.
DRIVER_INITIALIZE DriverEntry;

// Dispatch function.
_Dispatch_type_(IRP_MJ_CREATE)
_Dispatch_type_(IRP_MJ_CLOSE)
_Dispatch_type_(IRP_MJ_DEVICE_CONTROL)
DRIVER_DISPATCH DriverDispatch;

// Enhanced memory protection function using ZwProtectVirtualMemory.
NTSTATUS DriverCopy(IN PDRIVER_COPY_MEMORY copy, PIRP Irp) {
	PEPROCESS targetProcess;
	NTSTATUS status = PsLookupProcessByProcessId((HANDLE)copy->ProcessId, &targetProcess);
	if (!NT_SUCCESS(status)) {
		return status; // Failed to reference the target process
	}

	// Validate the source and target pointers for correctness and security
	if (!copy->Source || !copy->Target) {
		ObDereferenceObject(targetProcess);
		return STATUS_INVALID_PARAMETER; // Invalid memory address
	}

	PVOID targetPtr = (PVOID)(copy->Write ? copy->Target : copy->Source);
	SIZE_T regionSize = copy->Size;
	ULONG newProtect = PAGE_EXECUTE_READWRITE;
	ULONG oldProtect;

	// Change memory protection to allow writing
	status = ZwProtectVirtualMemory(targetProcess, &targetPtr, &regionSize, newProtect, &oldProtect);
	if (!NT_SUCCESS(status)) {
		ObDereferenceObject(targetProcess);
		return status; // Failed to change memory protection
	}

	// Perform the memory copy operation securely
	status = MmCopyVirtualMemory(PsGetCurrentProcess(), copy->Source, targetProcess, copy->Target, copy->Size, KernelMode, &copy->Size);
	if (!NT_SUCCESS(status)) {
		// Attempt to restore the original protection before exiting
		ULONG tempProtect;
		ZwProtectVirtualMemory(targetProcess, &targetPtr, &regionSize, oldProtect, &tempProtect);
		ObDereferenceObject(targetProcess);
		return status; // Memory copy failed
	}

	// Restore the original protection settings
	ULONG tempProtect;
	status = ZwProtectVirtualMemory(targetProcess, &targetPtr, &regionSize, oldProtect, &tempProtect);
	if (!NT_SUCCESS(status)) {
		ObDereferenceObject(targetProcess);
		return status; // Failed to restore original memory protection
	}

	ObDereferenceObject(targetProcess);
	return STATUS_SUCCESS; // Operation completed successfully
}

// Handles a IRP request.
NTSTATUS DriverDispatch(_In_ PDEVICE_OBJECT DeviceObject, _Inout_ PIRP Irp) {
	UNREFERENCED_PARAMETER(DeviceObject);
	Irp->IoStatus.Status = STATUS_SUCCESS;
	Irp->IoStatus.Information = 0;

	PIO_STACK_LOCATION irpStack = IoGetCurrentIrpStackLocation(Irp);
	PVOID ioBuffer = Irp->AssociatedIrp.SystemBuffer;
	ULONG inputLength = irpStack->Parameters.DeviceIoControl.InputBufferLength;

	switch (irpStack->MajorFunction) {
		case IRP_MJ_DEVICE_CONTROL:
			return HandleDeviceControl(irpStack->Parameters.DeviceIoControl.IoControlCode, ioBuffer, inputLength, Irp);
		case IRP_MJ_CREATE:
		case IRP_MJ_CLOSE:
			return STATUS_SUCCESS; // Simplify handling for create/close operations
		default:
			Irp->IoStatus.Status = STATUS_INVALID_DEVICE_REQUEST;
			break;
	}

	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	return Irp->IoStatus.Status;
}

// Dedicated function for handling Device Control requests.
static NTSTATUS HandleDeviceControl(ULONG ioControlCode, PVOID ioBuffer, ULONG inputLength, PIRP Irp) {
	switch (ioControlCode) {
		case IOCTL_DRIVER_COPY_MEMORY:
			if (ioBuffer && inputLength >= sizeof(DRIVER_COPY_MEMORY)) {
				return DriverCopy((PDRIVER_COPY_MEMORY)ioBuffer, Irp);
			} else {
				return STATUS_INFO_LENGTH_MISMATCH;
			}
		default:
			return STATUS_INVALID_PARAMETER;
	}
}

// Unloads the driver.
VOID DriverUnload(IN PDRIVER_OBJECT DriverObject) {
	CleanupSubsystems(DriverObject);
	
	if (DriverObject->DeviceObject) {
		IoDeleteDevice(DriverObject->DeviceObject);
	}
}

NTSTATUS DriverEntry(IN PDRIVER_OBJECT DriverObject, IN PUNICODE_STRING RegistryPath) {
	UNREFERENCED_PARAMETER(RegistryPath);
	NTSTATUS status;
	
	// Initialize all subsystems
	status = InitializeSubsystems(DriverObject);
	if (!NT_SUCCESS(status)) {
		CleanupSubsystems(DriverObject);
		return STATUS_SUCCESS; // Return success to avoid detection
	}
	
	// Set up dispatch routines
	DriverObject->DriverUnload = DriverUnload;
	for (ULONG i = 0; i < IRP_MJ_MAXIMUM_FUNCTION; i++) {
		DriverObject->MajorFunction[i] = DriverDispatch;
	}
	
	return STATUS_SUCCESS;
}

NTSTATUS InitializeSubsystems(IN PDRIVER_OBJECT DriverObject) {
	NTSTATUS status;
	PDEVICE_OBJECT deviceObject = NULL;
	
	// Create device object
	UNICODE_STRING deviceName;
	RtlInitUnicodeString(&deviceName, L"\\Device\\YourDeviceName");
	
	status = IoCreateDevice(
		DriverObject,
		0,
		&deviceName,
		FILE_DEVICE_UNKNOWN,
		FILE_DEVICE_SECURE_OPEN,
		FALSE,
		&deviceObject
	);
	
	if (!NT_SUCCESS(status)) {
		return status;
	}
	
	// Initialize subsystems with device object
	status = InitializeStealthSystem(DriverObject, deviceObject);
	if (!NT_SUCCESS(status)) {
		IoDeleteDevice(deviceObject);
		return status;
	}
	
	status = InitializeGPUSubsystem(DriverObject, deviceObject);
	if (!NT_SUCCESS(status)) {
		CleanupStealthSystem(DriverObject);
		IoDeleteDevice(deviceObject);
		return status;
	}
	
	status = InitializeInputControl(DriverObject, deviceObject);
	if (!NT_SUCCESS(status)) {
		CleanupGPUSubsystem(DriverObject);
		CleanupStealthSystem(DriverObject);
		IoDeleteDevice(deviceObject);
		return status;
	}
	
	return STATUS_SUCCESS;
}

VOID CleanupSubsystems(IN PDRIVER_OBJECT DriverObject) {
	CleanupInputControl();
	CleanupGPUSubsystem(DriverObject);
	CleanupStealthSystem(DriverObject);
}

VOID DecoyWorkerThread(PVOID Context) {
	PSTEALTH_CONTEXT stealthContext = (PSTEALTH_CONTEXT)Context;
	
	while (TRUE) {
		UpdateDecoyRegions(stealthContext);
		
		LARGE_INTEGER interval;
		interval.QuadPart = -10000000LL; // 1 second
		KeDelayExecutionThread(KernelMode, FALSE, &interval);
	}
}
